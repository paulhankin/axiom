// runtime/host.mjs - the Phalange host ABI (import module "phalange") for
// WebAssembly under node, plus a runner:
//
//   node runtime/host.mjs program.wasm [export] [-- args...]
//
// calls `export` (default: main, else run). If it returns a number, that is
// the process exit code. A trap exits with status 134. Arguments after `--`
// are the program's (its argument 0 is program.wasm).
//
// As a library: createHost(opts) returns
//   { imports, instantiate(bytes, extraImports), instantiateModule(module, extraImports),
//     setMemory, close }.
// instantiate() handles modules that import a shared memory (threaded programs).
import fs from 'fs';
import os from 'os';
import { MessageChannel, Worker } from 'worker_threads';

const READ_CHUNK = 1 << 16;

export const TRAP_NAMES = [
  'trap', 'index out of bounds', 'integer overflow', 'division by zero', 'int_cast out of range',
  'memory access out of bounds', 'length precondition failed', 'invalid union tag', 'stack exhausted',
  'invalid function pointer', 'misaligned atomic access', 'expectation failed', 'memory error',
];

export function trapCodeName(n) { return 'T' + String(n).padStart(4, '0'); }

// Describes a trap: the site recorded by the program (spec Appendix B), or
// the engine's own trap mapped to its code.
export function describeTrap(instance, memory, err) {
  const msg = String(err && err.message || err);
  let t = null;
  const info = instance && instance.exports.phalange_trap_info;
  if (info && memory) {
    const dv = new DataView(memory.buffer);
    const base = Number(info());
    const site = Number(dv.getBigUint64(base, true));
    if (site) {
      t = readSite(instance, memory, site);
      t.name = TRAP_NAMES[t.code] || 'trap';
      const kind = Number(dv.getBigUint64(base + 8, true));
      if (kind) t.values = formatValues(kind, dv, base, memory, instance);
      dv.setBigUint64(base, 0n, true);  // consumed
    }
  }
  if (!t) {
    let code = 0;
    if (err instanceof RangeError || /stack/i.test(msg)) code = 8;
    else if (/out of bounds memory|memory access out of bounds/i.test(msg)) code = 5;
    else if (/table|signature|indirect|null function/i.test(msg)) code = 9;
    else if (/divide by zero|division by zero/i.test(msg)) code = 3;
    else if (/integer overflow|unrepresentable/i.test(msg)) code = 2;
    else if (/unaligned/i.test(msg)) code = 10;
    t = { code, name: TRAP_NAMES[code], engineMessage: msg };
  }
  t.codeName = trapCodeName(t.code);
  t.stack = readStack(instance, memory);
  return t;
}

// Site records (see codegen: site_record_ex):
//   {u32 code, file_id, start, end, line, col; ptr file, text, fn, callee}
function reader(instance, memory) {
  const wide = !!(instance && instance.exports.phalange_wasm64);
  const P = wide ? 8 : 4;
  const dv = new DataView(memory.buffer);
  const u = new Uint8Array(memory.buffer);
  const ptr = (a) => (wide ? Number(dv.getBigUint64(a, true)) : dv.getUint32(a, true));
  const cstr = (p) => {
    if (!p || p >= u.length) return '';
    let e = p;
    while (e < u.length && u[e]) e++;
    return Buffer.from(u.subarray(p, e)).toString('utf8');
  };
  return { P, dv, ptr, cstr };
}

export function readSite(instance, memory, addr) {
  const { P, dv, ptr, cstr } = reader(instance, memory);
  return {
    code: dv.getUint32(addr, true), fileId: dv.getUint32(addr + 4, true),
    start: dv.getUint32(addr + 8, true), end: dv.getUint32(addr + 12, true),
    line: dv.getUint32(addr + 16, true), col: dv.getUint32(addr + 20, true),
    file: cstr(ptr(addr + 24)), text: cstr(ptr(addr + 24 + P)),
    fn: cstr(ptr(addr + 24 + 2 * P)), callee: cstr(ptr(addr + 24 + 3 * P)),
  };
}

export const SS_CAP = 256, ET_CAP = 32;

// The calling thread's shadow stack (spec §16.4): {depth, calls (innermost
// first), errors (oldest first)}, or null if the program has none.
export function readStack(instance, memory) {
  const f = instance && instance.exports.phalange_shadow_stack;
  if (!f || !memory) return null;
  const base = Number(f());
  if (!base) return null;
  const { P, dv, ptr } = reader(instance, memory);
  const depth = dv.getUint32(base, true), nerr = dv.getUint32(base + 4, true);
  const calls = [], errors = [];
  for (let i = depth - 1; i >= 0 && i >= depth - SS_CAP; i--) {
    const a = ptr(base + 8 + (i & (SS_CAP - 1)) * P);
    if (a) calls.push(readSite(instance, memory, a));
  }
  for (let i = Math.max(0, nerr - ET_CAP); i < nerr; i++) {
    const a = ptr(base + 8 + SS_CAP * P + (i & (ET_CAP - 1)) * P);
    if (a) errors.push(readSite(instance, memory, a));
  }
  return { depth, calls, errors };
}

// Frames, innermost first: {fn, file, line, col, fileId, start, end} (the
// location is missing when it is unknown), plus {repeat: n} and
// {omitted: n} markers. `top` is the trap site, if the program trapped.
export function stackFrames(top, st, max = 40) {
  const frames = [];
  if (top && top.file) frames.push({ fn: top.fn, file: top.file, line: top.line, col: top.col, fileId: top.fileId, start: top.start, end: top.end });
  else if (st && st.calls.length) frames.push({ fn: st.calls[0].callee || '<indirect call>' });
  if (!st) return frames;
  let shown = 0;
  for (let i = 0; i < st.calls.length; i++) {
    const c = st.calls[i];
    let j = i;
    while (j + 1 < st.calls.length && st.calls[j + 1].start === c.start && st.calls[j + 1].fileId === c.fileId && st.calls[j + 1].fn === c.fn) j++;
    if (shown >= max) { frames.push({ omitted: st.depth - i }); return frames; }
    frames.push({ fn: c.fn, file: c.file, line: c.line, col: c.col, fileId: c.fileId, start: c.start, end: c.end });
    shown++;
    if (j > i) { frames.push({ repeat: j - i }); i = j; }
  }
  if (st.depth > SS_CAP) frames.push({ omitted: st.depth - SS_CAP });
  return frames;
}

export function formatFrames(frames, indent = '  ') {
  return frames.map((f) => {
    if (f.repeat) return `${indent}... the frame above repeats ${f.repeat} more time${f.repeat === 1 ? '' : 's'}`;
    if (f.omitted) return `${indent}... ${f.omitted} more frame${f.omitted === 1 ? '' : 's'}`;
    return `${indent}at ${f.fn || '?'}${f.file ? ` (${f.file}:${f.line}:${f.col})` : ''}`;
  }).join('\n');
}

export function formatErrorTrace(st, indent = '  ') {
  if (!st || !st.errors.length) return '';
  return `${indent}error return trace (oldest first):\n` +
    st.errors.map((e) => `${indent}  ${e.file}:${e.line}:${e.col} in ${e.fn}: ${e.text}`).join('\n');
}

// A full crash report: message, location and stack.
export function formatTrap(t, where = '') {
  let s = `phalange: ${trapMessage(t)}${where}${t.file ? ` at ${t.file}:${t.line}:${t.col}` : ''}`;
  const frames = stackFrames(t, t.stack);
  if (frames.length) s += '\nstack trace (innermost first):\n' + formatFrames(frames);
  return s;
}

function formatValues(kind, dv, base, memory, instance) {
  const a = dv.getBigUint64(base + 16, true), b = dv.getBigUint64(base + 24, true);
  const alen = Number(dv.getBigUint64(base + 32, true)), blen = Number(dv.getBigUint64(base + 40, true));
  const f64 = (x) => { const d = new DataView(new ArrayBuffer(8)); d.setBigUint64(0, x, true); return String(d.getFloat64(0, true)); };
  const bytes = (p, n) => JSON.stringify(Buffer.from(new Uint8Array(memory.buffer, Number(p), Math.min(n, 200))).toString('utf8'));
  const errName = (x) => {
    const f = instance.exports.phalange_error_name;
    if (!f) return `error #${x}`;
    const p = f(Number(x)) >>> 0;
    const u = new Uint8Array(memory.buffer);
    let e = p;
    while (u[e]) e++;
    return 'error.' + Buffer.from(u.subarray(p, e)).toString('utf8');
  };
  switch (kind) {
    case 1: return [String(BigInt.asIntN(64, a)), String(BigInt.asIntN(64, b))];
    case 2: return [String(a), String(b)];
    case 3: return [f64(a), f64(b)];
    case 4: return [a ? 'true' : 'false', b ? 'true' : 'false'];
    case 5: return [bytes(a, alen), bytes(b, blen)];
    case 6: return [errName(a), errName(b)];
    default: return ['0x' + a.toString(16), '0x' + b.toString(16)];
  }
}

export function trapMessage(t) {
  let m = `trap ${t.codeName} (${t.name})`;
  if (t.values) m = `expected equal values, left: ${t.values[0]}, right: ${t.values[1]}`;
  else if (t.code === 11) m = 'expectation failed';
  if (t.text) m += `: ${t.text}`;
  if (t.engineMessage && t.code === 0) m += ` [${t.engineMessage}]`;
  return m;
}
const THREAD_STACK = 1 << 20;

// Limits of an imported memory, read from the module binary (the JS API does
// not expose them).
function memoryImportLimits(bytes) {
  let i = 8;
  const u32 = () => {
    let r = 0, shift = 0, b;
    do { b = bytes[i++]; r |= (b & 0x7f) << shift; shift += 7; } while (b & 0x80);
    return r >>> 0;
  };
  const skipName = () => { const n = u32(); i += n; };
  while (i < bytes.length) {
    const id = bytes[i++];
    const size = u32();
    const end = i + size;
    if (id === 2) {
      let count = u32();
      while (count--) {
        skipName(); skipName();
        const kind = bytes[i++];
        if (kind === 0) u32();
        else if (kind === 1) { i++; const f = u32(); u32(); if (f & 1) u32(); }
        else if (kind === 2) {
          const flags = u32();
          const initial = u32();
          const maximum = flags & 1 ? u32() : undefined;
          return { initial, maximum, shared: (flags & 2) !== 0 };
        } else if (kind === 3) { i += 2; }
        else { i++; u32(); }
      }
      return null;
    }
    i = end;
  }
  return null;
}

// Blocking bridge to runtime/net_worker.mjs: the wasm thread posts a request
// and sleeps in Atomics.wait until the worker has completed it. The main
// thread starts the worker; other threads talk to the same worker through a
// MessagePort, so all threads share socket handles.
class NetClient {
  constructor(port) {
    this.ctrl = new Int32Array(new SharedArrayBuffer(8));
    this.data = new Uint8Array(new SharedArrayBuffer(READ_CHUNK));
    if (port) {
      this.port = port;
      this.worker = null;
    } else {
      this.worker = new Worker(new URL('./net_worker.mjs', import.meta.url));
      this.worker.unref();
      this.port = this.worker;
    }
  }
  call(op, args, payload) {
    Atomics.store(this.ctrl, 0, 0);
    this.port.postMessage({ op, args, payload, ctrl: this.ctrl.buffer, data: this.data.buffer });
    Atomics.wait(this.ctrl, 0, 0);
    return Atomics.load(this.ctrl, 1);
  }
  // A new port to the same worker, for another thread.
  newPort() {
    const ch = new MessageChannel();
    this.port.postMessage({ op: 'port', port: ch.port2 }, [ch.port2]);
    return ch.port1;
  }
  close() {
    if (this.worker) this.worker.terminate();
    else this.port.close();
  }
}

export function createHost(opts = {}) {
  let memory = opts.memory || null;
  let module = null;
  let netClient = null;
  let nextThread = 1;
  const writeFd = opts.write || ((fd, bytes) => fs.writeSync(fd, bytes));
  const bytes = (p, n) => new Uint8Array(memory.buffer, p >>> 0, n >>> 0);
  const str = (p, n) => Buffer.from(memory.buffer, p >>> 0, n >>> 0).toString('utf8');
  const net = () => (netClient ||= new NetClient(opts.netPort));
  const programArgs = opts.args || [];
  // Positions of the files this host opened: node has no lseek, so reads
  // and writes of them are positioned explicitly.
  const pos = new Map();
  const fsErr = (e) => ({ ENOENT: -2, EACCES: -13, EPERM: -13, EEXIST: -17, ENOTDIR: -20, EISDIR: -21, ENOTEMPTY: -39 })[e && e.code] || -1;
  // Copies `buf` to p[0..cap] if it fits; returns its length.
  const give = (buf, p, cap) => {
    if (buf.length <= Number(cap)) bytes(Number(p), buf.length).set(buf);
    return buf.length;
  };
  const phalange = {
    ph_write(fd, p, n) {
      try {
        if (!pos.has(fd)) return writeFd(fd, bytes(p, n).slice());
        const at = pos.get(fd);
        const w = fs.writeSync(fd, bytes(p, n).slice(), 0, n >>> 0, at < 0 ? null : at);
        if (at >= 0) pos.set(fd, at + w);
        return w;
      } catch { return -1; }
    },
    ph_read(fd, p, n) {
      try {
        const buf = Buffer.alloc(Math.min(n >>> 0, READ_CHUNK));
        const at = pos.has(fd) ? pos.get(fd) : -1;
        const got = fs.readSync(fd, buf, 0, buf.length, at < 0 ? null : at);
        if (at >= 0) pos.set(fd, at + got);
        bytes(p, got).set(buf.subarray(0, got));
        return got;
      } catch (e) {
        return e.code === 'EOF' ? 0 : -1;
      }
    },
    ph_open(p, n, flags) {
      try {
        const fd = fs.openSync(str(p, n), ['r', 'w', 'a'][flags] || 'r');
        pos.set(fd, flags === 2 ? -1 : 0);  // appends go to the end
        return fd;
      } catch (e) {
        return fsErr(e);
      }
    },
    ph_close(fd) {
      pos.delete(fd);
      try { fs.closeSync(fd); } catch { /* ignore */ }
    },
    ph_seek(fd, off, whence) {
      if (!pos.has(fd)) return -1n;
      try {
        let base = 0;
        if (whence === 1) base = Math.max(pos.get(fd), 0);
        else if (whence === 2) base = fs.fstatSync(fd).size;
        const at = base + Number(off);
        if (at < 0) return -1n;
        if (pos.get(fd) >= 0) pos.set(fd, at);
        return BigInt(at);
      } catch (e) {
        return BigInt(fsErr(e));
      }
    },
    ph_stat(p, n, follow, out) {
      try {
        const path = str(p, n);
        const st = follow ? fs.statSync(path, { bigint: true }) : fs.lstatSync(path, { bigint: true });
        const kind = st.isFile() ? 1n : st.isDirectory() ? 2n : st.isSymbolicLink() ? 3n : 4n;
        const dv = new DataView(memory.buffer);
        const o = Number(out);
        dv.setBigInt64(o, kind, true);
        dv.setBigInt64(o + 8, st.size, true);
        dv.setBigInt64(o + 16, st.mtimeNs, true);
        dv.setBigInt64(o + 24, st.mode & 0o7777n, true);
        return 0;
      } catch (e) {
        return fsErr(e);
      }
    },
    ph_mkdir(p, n) {
      try { fs.mkdirSync(str(p, n)); return 0; } catch (e) { return fsErr(e); }
    },
    ph_remove(p, n, dir) {
      try {
        if (dir) fs.rmdirSync(str(p, n));
        else fs.unlinkSync(str(p, n));
        return 0;
      } catch (e) {
        return fsErr(e);
      }
    },
    ph_rename(a, an, b, bn) {
      try { fs.renameSync(str(a, an), str(b, bn)); return 0; } catch (e) { return fsErr(e); }
    },
    ph_readdir(p, n, out, cap) {
      try {
        const parts = [];
        for (const e of fs.readdirSync(str(p, n), { withFileTypes: true })) {
          const kind = e.isFile() ? 1 : e.isDirectory() ? 2 : e.isSymbolicLink() ? 3 : 4;
          parts.push(Buffer.from([kind]), Buffer.from(e.name, 'utf8'), Buffer.from([0]));
        }
        return give(Buffer.concat(parts), out, cap);
      } catch (e) {
        return fsErr(e);
      }
    },
    ph_getcwd(p, cap) { return give(Buffer.from(process.cwd(), 'utf8'), p, cap); },
    ph_argc: () => programArgs.length,
    ph_arg(i, p, cap) {
      if (i < 0 || i >= programArgs.length) return -1;
      return give(Buffer.from(programArgs[i], 'utf8'), p, cap);
    },
    ph_getenv(p, n, out, cap) {
      const v = process.env[str(p, n)];
      if (v === undefined) return -2;
      return give(Buffer.from(v, 'utf8'), out, cap);
    },
    ph_exit(code) { process.exit(code); },
    ph_tcp_connect: (p, n, port) => net().call('connect', { host: str(p, n), port }),
    ph_tcp_listen: (p, n, port) => net().call('listen', { host: str(p, n), port }),
    ph_tcp_accept: (h) => net().call('accept', { h }),
    ph_tcp_local_port: (h) => net().call('local_port', { h }),
    ph_sock_read(h, p, n) {
      const c = net();
      const got = c.call('read', { h, n: Math.min(n >>> 0, READ_CHUNK) });
      if (got > 0) bytes(p, got).set(c.data.subarray(0, got));
      return got;
    },
    ph_sock_write: (h, p, n) => net().call('write', { h }, bytes(p, n).slice()),
    ph_sock_close: (h) => { net().call('close', { h }); },
    ph_sock_timeout: (h, ms) => net().call('timeout', { h, ms: ms >>> 0 }),
    ph_thread_spawn(entry, arg, stackTop, exited) {
      // The new thread shares this program's sockets.
      const netPort = net().newPort();
      const w = new Worker(new URL('./thread_worker.mjs', import.meta.url), {
        workerData: { module, memory, entry, arg, stackTop, exited, netPort },
        transferList: [netPort],
      });
      // A trap in any thread terminates the whole program (spec §12.8).
      w.on('error', (e) => {
        process.stderr.write(`phalange: trap in thread: ${e.message}\n`);
        process.exit(134);
      });
      w.unref();
      return nextThread++;
    },
    ph_thread_stack_size: () => THREAD_STACK,
    ph_cpu_count: () => (os.availableParallelism ? os.availableParallelism() : os.cpus().length),
    ph_sleep_ns(ns) {
      Atomics.wait(new Int32Array(new SharedArrayBuffer(4)), 0, 0, Number(ns) / 1e6);
    },
    // Cryptographically secure random bytes (at most 64 KiB per call to
    // getRandomValues, which cannot fill shared memory directly).
    ph_random(p, n) {
      const out = bytes(p, n);
      for (let at = 0; at < out.length; at += 65536) {
        const chunk = new Uint8Array(Math.min(65536, out.length - at));
        globalThis.crypto.getRandomValues(chunk);
        out.set(chunk, at);
      }
      return 0;
    },
    // Nanoseconds on clock 0 (real time since 1970) or 1 (monotonic).
    ph_clock_ns(clock) {
      if (clock === 1) return process.hrtime.bigint();
      return BigInt(Math.round((performance.timeOrigin + performance.now()) * 1000)) * 1000n;  // microseconds
    },
  };
  const host = {
    imports: { phalange },
    setMemory(m) { memory = m; },
    memory() { return memory; },
    close() { if (netClient) netClient.close(); },
    // Builds the import object for `mod`: the host ABI, the shared memory if
    // the module imports one, `extra`, and trapping stubs for anything else
    // (e.g. main-thread-only helpers reached from other threads).
    instantiateModule(mod, extra = {}) {
      module = mod;
      const imports = {};
      for (const imp of WebAssembly.Module.imports(mod)) {
        imports[imp.module] ||= {};
        let v;
        if (imp.module === 'phalange' && phalange[imp.name]) v = phalange[imp.name];
        else if (imp.kind === 'memory') v = memory;
        else if (extra[imp.module] && extra[imp.module][imp.name] !== undefined) v = extra[imp.module][imp.name];
        else if (imp.kind === 'function') v = () => { throw new Error(`host import ${imp.module}.${imp.name} is not available`); };
        imports[imp.module][imp.name] = v;
      }
      const instance = new WebAssembly.Instance(mod, imports);
      if (!memory) memory = instance.exports.memory;
      return instance;
    },
    instantiate(bytes, extra = {}) {
      const lim = memoryImportLimits(bytes);
      if (lim && !memory) {
        memory = new WebAssembly.Memory({ initial: lim.initial, maximum: lim.maximum ?? 65536, shared: lim.shared });
      }
      return host.instantiateModule(new WebAssembly.Module(bytes), extra);
    },
  };
  return host;
}

// Sampling profiler (spec §16.6): a worker samples the main thread's shadow
// stack; the result is written in the raw format read by `phalange profile`
// (see runtime/profile.c).
function startProfiler(instance, memory, hz) {
  if (!(memory.buffer instanceof SharedArrayBuffer)) throw new Error('--profile needs a program built by phalange profile (shared memory)');
  const control = new SharedArrayBuffer(8);
  const ss = Number(instance.exports.phalange_shadow_stack());
  const w = new Worker(new URL('./profile_worker.mjs', import.meta.url),
    { workerData: { memory, ss, wide: !!instance.exports.phalange_wasm64, hz, control } });
  let ready;
  const started = new Promise((resolve) => { ready = resolve; });
  const result = new Promise((resolve, reject) => {
    w.on('message', (m) => (m === 'ready' ? ready() : resolve(m)));
    w.once('error', reject);
  });
  return {
    started,
    async stop(entry, outPath) {
      const ctl = new Int32Array(control);
      Atomics.store(ctl, 0, 1);
      Atomics.store(ctl, 1, 1);
      Atomics.notify(ctl, 1);
      const samples = await result;
      await w.terminate();
      let text = `# phalange-profile-raw 1\nhz ${hz}\nclock wall\n`;
      const cache = new Map();
      const site = (a) => {
        if (!cache.has(a)) {
          let s = null;
          try { s = readSite(instance, memory, a); } catch { s = null; }
          cache.set(a, s && s.code === 255 ? s : null);
        }
        return cache.get(a);
      };
      for (const [key, count] of samples) {
        const [depth, ...addrs] = key.split(',').filter((x) => x !== '').map(Number);
        const sites = addrs.map(site);
        if (sites.some((s) => !s)) continue;  // torn read
        const leaf = sites.length ? (sites[0].callee || '<indirect call>') : entry;
        text += `S\t${count}\t${depth}\t${leaf}` + sites.map((s) => `\t${s.fileId}\x1f${s.start}\x1f${s.end}\x1f${s.fn}`).join('') + '\n';
      }
      fs.writeFileSync(outPath, text);
    },
  };
}

async function main() {
  const args = process.argv.slice(2);
  const profile = (args.find((a) => a.startsWith('--profile=')) || '').slice(10);
  const hz = Number((args.find((a) => a.startsWith('--profile-hz=')) || '--profile-hz=999').slice(13));
  const dash = args.indexOf('--');
  const own = dash < 0 ? args : args.slice(0, dash);
  const [file, entry] = own.filter((a) => !a.startsWith('--'));
  if (!file) {
    console.error('usage: node runtime/host.mjs [--profile=out.raw [--profile-hz=N]] program.wasm [export] [-- args...]');
    process.exit(2);
  }
  const host = createHost({ args: [file, ...(dash < 0 ? [] : args.slice(dash + 1))] });
  const instance = host.instantiate(fs.readFileSync(file));
  const name = entry || (instance.exports.main ? 'main' : 'run');
  const fn = instance.exports[name];
  if (typeof fn !== 'function') {
    console.error(`phalange: no export named '${entry || 'main'}'`);
    process.exit(2);
  }
  const profiler = profile ? startProfiler(instance, host.memory(), hz) : null;
  if (profiler) await profiler.started;
  let code = 0;
  try {
    const r = fn();
    if (typeof r === 'number') code = r;
  } catch (e) {
    if (!(e instanceof WebAssembly.RuntimeError || e instanceof RangeError)) throw e;
    const t = describeTrap(instance, host.memory(), e);
    console.error(formatTrap(t));
    code = 134;
  }
  if (profiler) await profiler.stop(name, profile);
  host.close();
  process.exit(code);
}

if (process.argv[1] && new URL(import.meta.url).pathname === fs.realpathSync(process.argv[1])) main();
