// runtime/net_worker.mjs - owns node's (asynchronous) sockets on behalf of
// the (synchronous) wasm threads; see NetClient in host.mjs.
//
// One worker serves every thread of a program, so socket handles are shared
// between threads. Each request carries the caller's own shared buffers:
// ctrl[0] is set to 1 (and notified) when the request completes, with its
// result in ctrl[1]; read data goes into `data`. Threads reach the worker
// through MessagePorts sent with a 'port' message.
import { parentPort } from 'worker_threads';
import child_process from 'child_process';
import dgram from 'dgram';
import dns from 'dns';
import fs from 'fs';
import net from 'net';
import os from 'os';
import path from 'path';

const handles = new Map();
let nextHandle = 3;

const ERR = -1, REFUSED = -2, ADDR_IN_USE = -3, BAD_ADDRESS = -4, TIMED_OUT = -5;

function reply(req, result) {
  const ctrl = new Int32Array(req.ctrl);
  Atomics.store(ctrl, 1, result);
  Atomics.store(ctrl, 0, 1);
  Atomics.notify(ctrl, 0);
}

function errCode(e) {
  switch (e && e.code) {
    case 'ECONNREFUSED': return REFUSED;
    case 'EADDRINUSE': return ADDR_IN_USE;
    case 'ENOTFOUND': case 'EAI_AGAIN': case 'ERR_INVALID_IP_ADDRESS': case 'ERR_INVALID_ADDRESS': return BAD_ADDRESS;
    default: return ERR;
  }
}

// Runs the queued waiters that can now make progress.
function wake(st) {
  const ws = st.waiters;
  st.waiters = [];
  for (const w of ws) w();
}

function addSocket(sock, readOnly = false) {
  const st = { kind: 'sock', sock, chunks: [], ended: false, failed: false, waiters: [], timeout: 0, readOnly };
  sock.on('data', (b) => { st.chunks.push(b); wake(st); });
  sock.on('end', () => { st.ended = true; wake(st); });
  sock.on('close', () => { st.ended = true; wake(st); });
  sock.on('error', () => { st.failed = true; st.ended = true; wake(st); });
  const h = nextHandle++;
  handles.set(h, st);
  return h;
}

const ops = {
  connect(req, { host, port }) {
    const sock = net.connect({ host: host || '127.0.0.1', port });
    const onError = (e) => reply(req, errCode(e));
    sock.once('error', onError);
    sock.once('connect', () => { sock.off('error', onError); reply(req, addSocket(sock)); });
  },
  listen(req, { host, port }) {
    const srv = net.createServer();
    const st = { kind: 'listener', srv, queue: [], waiters: [], closed: false };
    srv.on('connection', (s) => { st.queue.push(s); wake(st); });
    const onError = (e) => reply(req, errCode(e));
    srv.once('error', onError);
    srv.listen(port, host || undefined, () => {
      srv.off('error', onError);
      const h = nextHandle++;
      handles.set(h, st);
      reply(req, h);
    });
  },
  accept(req, { h }) {
    const st = handles.get(h);
    if (!st || st.kind !== 'listener') return reply(req, ERR);
    const attempt = () => {
      if (st.closed) reply(req, ERR);
      else if (st.queue.length) reply(req, addSocket(st.queue.shift()));
      else st.waiters.push(attempt);
    };
    attempt();
  },
  local_port(req, { h }) {
    const st = handles.get(h);
    if (!st) return reply(req, ERR);
    reply(req, st.kind === 'listener' ? st.srv.address().port : st.sock.localPort);
  },
  read(req, { h, n }) {
    const st = handles.get(h);
    if (!st || st.kind !== 'sock') return reply(req, ERR);
    const data = new Uint8Array(req.data);
    let timer = null;
    let done = false;
    const attempt = () => {
      if (done) return;
      if (st.chunks.length) {
        let got = 0;
        while (got < n && st.chunks.length) {
          const c = st.chunks[0];
          const take = Math.min(c.length, n - got);
          data.set(c.subarray(0, take), got);
          got += take;
          if (take === c.length) st.chunks.shift();
          else st.chunks[0] = c.subarray(take);
        }
        done = true;
        if (timer) clearTimeout(timer);
        reply(req, got);
      } else if (st.ended) {
        done = true;
        if (timer) clearTimeout(timer);
        reply(req, st.failed ? ERR : 0);
      } else {
        st.waiters.push(attempt);
      }
    };
    attempt();
    if (!done && st.timeout > 0) {
      timer = setTimeout(() => {
        if (done) return;
        done = true;
        st.waiters = st.waiters.filter((w) => w !== attempt);
        reply(req, TIMED_OUT);
      }, st.timeout);
    }
  },
  write(req, { h }, payload) {
    const st = handles.get(h);
    if (!st || st.kind !== 'sock' || st.failed) return reply(req, ERR);
    st.sock.write(payload, (err) => reply(req, err ? ERR : payload.length));
  },
  timeout(req, { h, ms }) {
    const st = handles.get(h);
    if (!st || (st.kind !== 'sock' && st.kind !== 'udp')) return reply(req, ERR);
    st.timeout = ms;
    reply(req, 0);
  },
  close(req, { h }) {
    const st = handles.get(h);
    handles.delete(h);
    if (st && st.kind === 'listener') {
      st.closed = true;
      st.srv.close();
      wake(st);
    } else if (st && st.kind === 'watch') {
      for (const w of st.watches.values()) w.close();
      st.closed = true;
      wake(st);
    } else if (st && st.kind === 'udp') {
      st.closed = true;
      st.sock.close();
      wake(st);
    } else if (st && st.kind === 'sock') {
      if (st.readOnly) st.sock.destroy();
      else st.sock.end();
    }
    reply(req, 0);
  },
  // Child processes (see ph_proc_spawn): the handle, and pipe handles in
  // data as three i32s.
  spawn(req, { argv, env, cwd, modes }) {
    const out = new Int32Array(req.data, 0, 3);
    out.fill(-1);
    const stdio = modes.map((m, k) => (m === 1 ? 'pipe' : m === 2 ? 'ignore' : k));
    let child;
    try {
      child = child_process.spawn(argv[0], argv.slice(1), { env, cwd: cwd || undefined, stdio });
    } catch (e) {
      return reply(req, spawnErr(e));
    }
    const st = { kind: 'proc', child, exited: false, code: 0, signal: 0, waiters: [] };
    child.on('exit', (code, signal) => {
      st.exited = true;
      st.code = code === null ? 0 : code;
      st.signal = signal ? os.constants.signals[signal] || 0 : 0;
      wake(st);
    });
    child.once('error', (e) => { if (!st.started) reply(req, spawnErr(e)); });
    child.once('spawn', () => {
      st.started = true;
      const h = nextHandle++;
      handles.set(h, st);
      const pipes = [child.stdin, child.stdout, child.stderr];
      for (let k = 0; k < 3; k++) if (modes[k] === 1) out[k] = addSocket(pipes[k], k !== 0);
      reply(req, h);
    });
  },
  // Addresses (records of 24 bytes: family, port, address, scope; see
  // runtime/native.ph), written to data.
  resolve(req, { name, max }) {
    dns.lookup(name, { all: true }, (err, addrs) => {
      if (err || !addrs.length) return reply(req, BAD_ADDRESS);
      const data = new Uint8Array(req.data);
      let n = 0;
      for (const a of addrs) {
        if (n >= max) break;
        data.set(addrRecord(a.address, 0), 24 * n);
        n++;
      }
      reply(req, n);
    });
  },
  connect_addr(req, { host, port }) {
    const sock = net.connect({ host, port });
    const onError = (e) => reply(req, errCode(e));
    sock.once('error', onError);
    sock.once('connect', () => { sock.off('error', onError); reply(req, addSocket(sock)); });
  },
  listen_addr(req, { host, port, path: p }) {
    const srv = net.createServer();
    const st = { kind: 'listener', srv, queue: [], waiters: [], closed: false };
    srv.on('connection', (c) => { st.queue.push(c); wake(st); });
    const onError = (e) => reply(req, errCode(e));
    srv.once('error', onError);
    const done = () => {
      srv.off('error', onError);
      const h = nextHandle++;
      handles.set(h, st);
      reply(req, h);
    };
    if (p) srv.listen(p, done);
    else srv.listen(port, host, done);
  },
  unix_connect(req, { path: p }) {
    const sock = net.connect({ path: p });
    const onError = (e) => reply(req, e && e.code === 'ENOENT' ? REFUSED : errCode(e));
    sock.once('error', onError);
    sock.once('connect', () => { sock.off('error', onError); reply(req, addSocket(sock)); });
  },
  sock_addr(req, { h, peer }) {
    const st = handles.get(h);
    if (!st) return reply(req, ERR);
    let host, port;
    if (st.kind === 'listener') ({ address: host, port } = st.srv.address() || {});
    else if (st.kind === 'udp') ({ address: host, port } = st.sock.address());
    else if (peer) { host = st.sock.remoteAddress; port = st.sock.remotePort; }
    else { host = st.sock.localAddress; port = st.sock.localPort; }
    if (!host) return reply(req, ERR);
    new Uint8Array(req.data).set(addrRecord(host, port), 0);
    reply(req, 0);
  },
  udp_open(req, { host, port, family }) {
    const sock = dgram.createSocket(family === 6 ? 'udp6' : 'udp4');
    const st = { kind: 'udp', sock, msgs: [], waiters: [], timeout: 0, closed: false };
    sock.on('message', (m, rinfo) => { st.msgs.push({ m, rinfo }); wake(st); });
    const onError = (e) => reply(req, errCode(e));
    sock.once('error', onError);
    sock.bind(port, host, () => {
      sock.off('error', onError);
      sock.on('error', () => {});
      const h = nextHandle++;
      handles.set(h, st);
      reply(req, h);
    });
  },
  udp_send(req, { h, host, port }, payload) {
    const st = handles.get(h);
    if (!st || st.kind !== 'udp') return reply(req, ERR);
    st.sock.send(payload, port, host, (err) => reply(req, err ? ERR : payload.length));
  },
  // the sender's record, then the datagram, in data
  udp_recv(req, { h, n }) {
    const st = handles.get(h);
    if (!st || st.kind !== 'udp') return reply(req, ERR);
    const data = new Uint8Array(req.data);
    let timer = null;
    let done = false;
    const attempt = () => {
      if (done) return;
      if (st.msgs.length) {
        const { m, rinfo } = st.msgs.shift();
        const k = Math.min(m.length, n);
        data.set(addrRecord(rinfo.address, rinfo.port), 0);
        data.set(m.subarray(0, k), 24);
        done = true;
        if (timer) clearTimeout(timer);
        reply(req, k);
      } else if (st.closed) {
        done = true;
        reply(req, ERR);
      } else {
        st.waiters.push(attempt);
      }
    };
    attempt();
    if (!done && st.timeout > 0) {
      timer = setTimeout(() => {
        if (done) return;
        done = true;
        st.waiters = st.waiters.filter((w) => w !== attempt);
        reply(req, TIMED_OUT);
      }, st.timeout);
    }
  },
  // readiness of the program's own handles: revents (u16 each) in data
  poll(req, { entries, timeout }) {
    const data = new Uint16Array(req.data, 0, entries.length);
    let timer = null;
    let done = false;
    const check = () => {
      let ready = 0;
      entries.forEach(({ h, events }, k) => {
        const st = handles.get(h);
        let r = 0;
        if (!st) r = 8;
        else if (st.kind === 'sock') {
          if ((events & 1) && (st.chunks.length || st.ended)) r |= 1;
          if (events & 4) r |= 4;
          if (st.failed) r |= 8;
          else if (st.ended && !st.chunks.length) r |= 16;
        } else if (st.kind === 'listener') {
          if ((events & 1) && st.queue.length) r |= 1;
        } else if (st.kind === 'udp') {
          if ((events & 1) && st.msgs.length) r |= 1;
          if (events & 4) r |= 4;
        } else if (st.kind === 'watch') {
          if ((events & 1) && st.queue.length) r |= 1;
        }
        data[k] = r;
        if (r) ready++;
      });
      return ready;
    };
    const attempt = () => {
      if (done) return;
      const n = check();
      if (n || timeout === 0) {
        done = true;
        if (timer) clearTimeout(timer);
        return reply(req, n);
      }
      for (const { h } of entries) {
        const st = handles.get(h);
        if (st && st.waiters && !st.waiters.includes(attempt)) st.waiters.push(attempt);
      }
    };
    attempt();
    if (!done && timeout > 0) {
      timer = setTimeout(() => {
        if (done) return;
        done = true;
        for (const { h } of entries) {
          const st = handles.get(h);
          if (st && st.waiters) st.waiters = st.waiters.filter((w) => w !== attempt);
        }
        reply(req, 0);
      }, timeout);
    }
  },
  // Watching, as inotify would report it (records of wd, mask, cookie,
  // len, name): 'change' is IN_MODIFY; 'rename' is IN_CREATE or IN_DELETE
  // (IN_DELETE_SELF for a watched file), by whether the path exists.
  watch_open(req) {
    const st = { kind: 'watch', watches: new Map(), queue: [], waiters: [], nextWd: 1, closed: false };
    const h = nextHandle++;
    handles.set(h, st);
    reply(req, h);
  },
  watch_add(req, { h, path: p }) {
    const st = handles.get(h);
    if (!st || st.kind !== 'watch') return reply(req, ERR);
    let isDir;
    try { isDir = fs.statSync(p).isDirectory(); } catch (e) { return reply(req, e.code === 'ENOENT' ? -2 : ERR); }
    const wd = st.nextWd++;
    const record = (mask, name) => {
      const nb = Buffer.from(name || '');
      const len = nb.length ? (nb.length + 4) & ~3 : 0;
      const r = Buffer.alloc(16 + len);
      r.writeInt32LE(wd, 0);
      r.writeUInt32LE(mask, 4);
      r.writeUInt32LE(len, 12);
      nb.copy(r, 16);
      st.queue.push(r);
      wake(st);
    };
    let w;
    try {
      w = fs.watch(p, (type, filename) => {
        const name = isDir ? filename || '' : '';
        if (type === 'change') return record(0x2, name);
        const exists = fs.existsSync(isDir && name ? path.join(p, name) : p);
        record(exists ? 0x100 : isDir ? 0x200 : 0x400, name);
      });
    } catch (e) {
      return reply(req, ERR);
    }
    w.on('error', () => {});
    st.watches.set(wd, w);
    reply(req, wd);
  },
  watch_remove(req, { h, wd }) {
    const st = handles.get(h);
    const w = st && st.kind === 'watch' && st.watches.get(wd);
    if (!w) return reply(req, ERR);
    w.close();
    st.watches.delete(wd);
    reply(req, 0);
  },
  watch_read(req, { h, n, timeout }) {
    const st = handles.get(h);
    if (!st || st.kind !== 'watch') return reply(req, ERR);
    const data = new Uint8Array(req.data);
    let timer = null;
    let done = false;
    const attempt = () => {
      if (done) return;
      if (st.queue.length || st.closed) {
        let got = 0;
        while (st.queue.length && got + st.queue[0].length <= n) {
          const r = st.queue.shift();
          data.set(r, got);
          got += r.length;
        }
        done = true;
        if (timer) clearTimeout(timer);
        reply(req, got);
      } else {
        st.waiters.push(attempt);
      }
    };
    attempt();
    if (!done && timeout >= 0) {
      timer = setTimeout(() => {
        if (done) return;
        done = true;
        st.waiters = st.waiters.filter((w) => w !== attempt);
        reply(req, 0);
      }, timeout);
    }
  },
  wait(req, { h, block }) {
    const st = handles.get(h);
    if (!st || st.kind !== 'proc') return reply(req, ERR);
    const out = new Int32Array(req.data, 0, 1);
    const attempt = () => {
      if (st.exited) {
        handles.delete(h);
        out[0] = st.signal || st.code;
        reply(req, st.signal ? 2 : 1);
      } else if (!block) {
        reply(req, 0);
      } else {
        st.waiters.push(attempt);
      }
    };
    attempt();
  },
  kill(req, { h, sig }) {
    const st = handles.get(h);
    if (!st || st.kind !== 'proc') return reply(req, ERR);
    reply(req, st.exited || st.child.kill(sig) ? 0 : ERR);
  },
};

// An address record for a textual address and a port.
function addrRecord(host, port) {
  const r = new Uint8Array(24);
  r[2] = (port >> 8) & 0xff;
  r[3] = port & 0xff;
  if (net.isIPv4(host)) {
    r[0] = 4;
    host.split('.').forEach((p, i) => { r[4 + i] = Number(p); });
    return r;
  }
  r[0] = 6;
  let h = host;
  const pct = h.indexOf('%');
  if (pct >= 0) h = h.slice(0, pct);
  if (h.includes('.')) {
    // ::ffff:1.2.3.4
    const k = h.lastIndexOf(':');
    const v4 = h.slice(k + 1).split('.').map(Number);
    h = h.slice(0, k + 1) + ((v4[0] << 8) | v4[1]).toString(16) + ':' + ((v4[2] << 8) | v4[3]).toString(16);
  }
  const [a, b] = h.split('::');
  const left = a ? a.split(':') : [];
  const right = b !== undefined && b ? b.split(':') : [];
  const groups = b === undefined ? left : [...left, ...Array(8 - left.length - right.length).fill('0'), ...right];
  groups.forEach((g, i) => {
    const v = parseInt(g || '0', 16);
    r[4 + 2 * i] = v >> 8;
    r[5 + 2 * i] = v & 0xff;
  });
  return r;
}

function spawnErr(e) {
  switch (e && e.code) {
    case 'ENOENT': return -2;
    case 'EACCES': case 'EPERM': return -13;
    default: return ERR;
  }
}

function serve(port) {
  port.on('message', (m) => {
    if (m.op === 'port') serve(m.port);
    else ops[m.op](m, m.args || {}, m.payload);
  });
}

serve(parentPort);
