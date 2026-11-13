// std/internal/runtime/memfs.mjs - a file system in memory, for a host that has
// no files of its own (a browser) or should not give its own to a program
// (host.mjs --files=memory): the part of node's synchronous `fs` interface
// that host.mjs's host functions use, with node's error codes (thrown as
// errors with a `code`), and the working directory. It needs nothing of
// node's, so a browser's host can use it as it is.
//
// Files, directories and symbolic links are nodes; a directory maps names
// to nodes (hard links are two names of one node). Paths are POSIX paths:
// relative ones are resolved against the working directory, `.` and `..`
// are followed, and so are symbolic links (at most 40 of them in a
// resolution, else ELOOP), but a final one only where node's call would.
// Descriptors are numbers from 3 up; each holds its node, so that a file
// removed while open stays readable. Times are nanoseconds (BigInt).

const S_IFREG = 0o100000, S_IFDIR = 0o040000, S_IFLNK = 0o120000;

function fail(code, what) {
  return Object.assign(new Error(`${code}: ${what}`), { code });
}

function nowNs() {
  const ms = (typeof performance !== 'undefined' && performance.timeOrigin) ? performance.timeOrigin + performance.now() : Date.now();
  return BigInt(Math.round(ms * 1000)) * 1000n;
}

// The flags of node's openSync as strings ('r', 'w+', 'ax', ...) or numbers
// (O_* of `constants` below): read, write, create, truncate, append, excl,
// directory, nofollow.
function openFlags(f) {
  if (typeof f === 'number') {
    const acc = f & 3;
    return { read: acc !== 1, write: acc !== 0, create: !!(f & 0o100), trunc: !!(f & 0o1000), append: !!(f & 0o2000),
      excl: !!(f & 0o200), dir: !!(f & 0o200000), nofollow: !!(f & 0o400000) };
  }
  const plus = f.includes('+'), k = f[0];
  return { read: k === 'r' || plus, write: k !== 'r' || plus, create: k !== 'r', trunc: k === 'w', append: k === 'a',
    excl: f.includes('x'), dir: false, nofollow: false };
}

class Stats {
  constructor(n, big) {
    const kind = n.kind === 'file' ? S_IFREG : n.kind === 'dir' ? S_IFDIR : S_IFLNK;
    const size = n.kind === 'file' ? n.size : n.kind === 'link' ? n.target.length : 4096;
    const num = (x) => (big ? BigInt(x) : Number(x));
    this.n = n;
    this.dev = num(1);
    this.ino = num(n.ino);
    this.mode = num(kind | n.mode);
    this.nlink = num(n.links);
    this.size = num(size);
    this.mtimeNs = n.mtime;
    this.atimeNs = n.atime;
    this.mtimeMs = Number(n.mtime / 1000000n);
  }
  isFile() { return this.n.kind === 'file'; }
  isDirectory() { return this.n.kind === 'dir'; }
  isSymbolicLink() { return this.n.kind === 'link'; }
}

class Dirent {
  constructor(name, n) { this.name = name; this.n = n; }
  isFile() { return this.n.kind === 'file'; }
  isDirectory() { return this.n.kind === 'dir'; }
  isSymbolicLink() { return this.n.kind === 'link'; }
}

export class MemFS {
  constructor() {
    this.nextIno = 1;
    this.root = this.node('dir', 0o755);
    this.root.parent = this.root;
    this.dir = '/';  // the working directory
    this.fds = new Map();
    this.nextFd = 3;
    this.constants = { O_RDONLY: 0, O_WRONLY: 1, O_RDWR: 2, O_CREAT: 0o100, O_EXCL: 0o200, O_TRUNC: 0o1000,
      O_APPEND: 0o2000, O_DIRECTORY: 0o200000, O_NOFOLLOW: 0o400000 };
    this.mkdirSync('/tmp', { mode: 0o1777 });
  }
  node(kind, mode) {
    const t = nowNs();
    const n = { kind, mode: mode & 0o7777, ino: this.nextIno++, links: 1, mtime: t, atime: t };
    if (kind === 'file') { n.data = new Uint8Array(0); n.size = 0; }
    if (kind === 'dir') n.entries = new Map();
    return n;
  }
  // The absolute path p names (joined to the working directory), as names.
  parts(p) {
    if (typeof p !== 'string' || p === '' || p.includes('\0')) throw fail('ENOENT', p);
    return (p[0] === '/' ? p : this.dir + '/' + p).split('/').filter((x) => x !== '' && x !== '.');
  }
  // {parent, name, node} for path p: the directory that holds its last name
  // (links on the way followed) and the node there, if any (a final link
  // followed if follow).
  lookup(p, follow, depth = 0) {
    const names = this.parts(p);
    let dir = this.root;
    for (let i = 0; i < names.length; i++) {
      const name = names[i], last = i === names.length - 1;
      if (dir.kind !== 'dir') throw fail('ENOTDIR', p);
      if (name === '..') {
        dir = dir.parent;
        if (last) return { parent: dir.parent, name: null, node: dir };
        continue;
      }
      let n = dir.entries.get(name);
      if (n && n.kind === 'link' && (!last || follow)) {
        if (depth >= 40) throw fail('ELOOP', p);
        const base = '/' + this.pathOf(dir);
        const r = this.lookup(n.target[0] === '/' ? n.target : base + '/' + n.target, true, depth + 1);
        if (last) return r;
        n = r.node;
        if (!n) throw fail('ENOENT', p);
      }
      if (last) return { parent: dir, name, node: n || null };
      if (!n) throw fail('ENOENT', p);
      dir = n;
    }
    return { parent: dir.parent, name: null, node: dir };  // the root, or a path ending in '..'
  }
  pathOf(dir) {
    const names = [];
    while (dir !== this.root) {
      const up = dir.parent;
      for (const [k, v] of up.entries) if (v === dir) { names.unshift(k); break; }
      dir = up;
    }
    return names.join('/');
  }
  existing(p, follow) {
    const r = this.lookup(p, follow);
    if (!r.node) throw fail('ENOENT', p);
    return r;
  }
  fd(fd) {
    const f = this.fds.get(fd);
    if (!f) throw fail('EBADF', String(fd));
    return f;
  }

  cwd() { return this.dir; }
  chdir(p) {
    const { node } = this.existing(p, true);
    if (node.kind !== 'dir') throw fail('ENOTDIR', p);
    this.dir = '/' + this.pathOf(node);
  }
  openSync(p, flags = 'r', mode = 0o666) {
    const o = openFlags(flags);
    let r = this.lookup(p, !o.nofollow);
    let n = r.node;
    if (n && o.create && o.excl) throw fail('EEXIST', p);
    if (n && n.kind === 'link') throw fail('ELOOP', p);
    if (!n) {
      if (!o.create || !r.name) throw fail('ENOENT', p);
      if (r.parent.kind !== 'dir') throw fail('ENOTDIR', p);
      n = this.node('file', mode);
      n.parent = r.parent;
      r.parent.entries.set(r.name, n);
      r.parent.mtime = nowNs();
    }
    if (n.kind === 'dir' && o.write) throw fail('EISDIR', p);
    if (o.dir && n.kind !== 'dir') throw fail('ENOTDIR', p);
    if (o.trunc && n.kind === 'file') { n.data = new Uint8Array(0); n.size = 0; n.mtime = nowNs(); }
    const fd = this.nextFd++;
    this.fds.set(fd, { node: n, pos: 0, read: o.read, write: o.write, append: o.append });
    return fd;
  }
  closeSync(fd) { this.fd(fd); this.fds.delete(fd); }
  readSync(fd, buf, off = 0, len = buf.length - off, pos = null) {
    const f = this.fd(fd), n = f.node;
    if (n.kind === 'dir') throw fail('EISDIR', String(fd));
    if (!f.read) throw fail('EBADF', String(fd));
    const at = pos === null || pos === undefined ? f.pos : Number(pos);
    const k = Math.max(0, Math.min(len, n.size - at));
    buf.set(n.data.subarray(at, at + k), off);
    if (pos === null || pos === undefined) f.pos += k;
    n.atime = nowNs();
    return k;
  }
  writeSync(fd, buf, off = 0, len = buf.length - off, pos = null) {
    const f = this.fd(fd), n = f.node;
    if (!f.write || n.kind !== 'file') throw fail('EBADF', String(fd));
    const at = f.append ? n.size : pos === null || pos === undefined ? f.pos : Number(pos);
    this.resize(n, Math.max(n.size, at + len));
    n.data.set(buf.subarray(off, off + len), at);
    if (pos === null || pos === undefined || f.append) f.pos = at + len;
    n.mtime = nowNs();
    return len;
  }
  resize(n, size) {
    if (size > n.data.length) {
      const d = new Uint8Array(Math.max(size, n.data.length * 2, 64));
      d.set(n.data.subarray(0, n.size));
      n.data = d;
    } else if (size < n.size) {
      n.data.fill(0, size, n.size);
    }
    n.size = size;
  }
  ftruncateSync(fd, len = 0) {
    const f = this.fd(fd);
    if (!f.write || f.node.kind !== 'file') throw fail('EINVAL', String(fd));
    this.resize(f.node, Number(len));
    f.node.mtime = nowNs();
  }
  fsyncSync(fd) { this.fd(fd); }
  fstatSync(fd, o = {}) { return new Stats(this.fd(fd).node, !!o.bigint); }
  statSync(p, o = {}) { return new Stats(this.existing(p, true).node, !!o.bigint); }
  lstatSync(p, o = {}) { return new Stats(this.existing(p, false).node, !!o.bigint); }
  mkdirSync(p, o = {}) {
    const r = this.lookup(p, false);
    if (r.node || !r.name) throw fail('EEXIST', p);
    if (r.parent.kind !== 'dir') throw fail('ENOTDIR', p);
    const d = this.node('dir', o.mode ?? 0o777);
    d.parent = r.parent;
    r.parent.entries.set(r.name, d);
    r.parent.mtime = nowNs();
  }
  rmdirSync(p) {
    const r = this.existing(p, false);
    if (r.node.kind !== 'dir') throw fail('ENOTDIR', p);
    if (r.node === this.root || !r.name) throw fail('EBUSY', p);
    if (r.node.entries.size) throw fail('ENOTEMPTY', p);
    r.parent.entries.delete(r.name);
    r.parent.mtime = nowNs();
  }
  unlinkSync(p) {
    const r = this.existing(p, false);
    if (r.node.kind === 'dir') throw fail('EISDIR', p);
    r.parent.entries.delete(r.name);
    r.node.links--;
    r.parent.mtime = nowNs();
  }
  renameSync(a, b) {
    const from = this.existing(a, false), to = this.lookup(b, false);
    if (!from.name || !to.name) throw fail('EBUSY', a);
    if (to.parent.kind !== 'dir') throw fail('ENOTDIR', b);
    if (from.node.kind === 'dir') {
      // (not into itself)
      for (let d = to.parent; ; d = d.parent) {
        if (d === from.node) throw fail('EINVAL', b);
        if (d === this.root) break;
      }
    }
    if (to.node && to.node !== from.node) {
      if (to.node.kind === 'dir' && from.node.kind !== 'dir') throw fail('EISDIR', b);
      if (to.node.kind !== 'dir' && from.node.kind === 'dir') throw fail('ENOTDIR', b);
      if (to.node.kind === 'dir' && to.node.entries.size) throw fail('ENOTEMPTY', b);
      to.node.links--;
    }
    from.parent.entries.delete(from.name);
    to.parent.entries.set(to.name, from.node);
    from.node.parent = from.node.kind === 'dir' ? to.parent : from.node.parent;
    from.parent.mtime = to.parent.mtime = nowNs();
  }
  readdirSync(p, o = {}) {
    const { node } = this.existing(p, true);
    if (node.kind !== 'dir') throw fail('ENOTDIR', p);
    const names = [...node.entries.keys()].sort();
    return o.withFileTypes ? names.map((k) => new Dirent(k, node.entries.get(k))) : names;
  }
  chmodSync(p, mode) { this.existing(p, true).node.mode = mode & 0o7777; }
  symlinkSync(target, p) {
    const r = this.lookup(p, false);
    if (r.node || !r.name) throw fail('EEXIST', p);
    const l = this.node('link', 0o777);
    l.target = target;
    l.parent = r.parent;
    r.parent.entries.set(r.name, l);
  }
  linkSync(a, b) {
    const from = this.existing(a, false), to = this.lookup(b, false);
    if (from.node.kind === 'dir') throw fail('EPERM', a);
    if (to.node || !to.name) throw fail('EEXIST', b);
    to.parent.entries.set(to.name, from.node);
    from.node.links++;
  }
  readlinkSync(p, o = {}) {
    const { node } = this.existing(p, false);
    if (node.kind !== 'link') throw fail('EINVAL', p);
    return o.encoding === 'buffer' ? new TextEncoder().encode(node.target) : node.target;
  }
  utimesSync(p, atime, mtime) {
    const { node } = this.existing(p, true);
    const ns = (s) => BigInt(Math.floor(s)) * 1000000000n + BigInt(Math.round((s - Math.floor(s)) * 1e9));
    node.atime = ns(atime);
    node.mtime = ns(mtime);
  }
}
