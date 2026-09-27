// runtime/host.mjs - the Phalange host ABI (import module "phalange") for
// WebAssembly under node, plus a runner:
//
//   node runtime/host.mjs program.wasm [export]
//
// calls `export` (default: main, else run). If it returns a number, that is
// the process exit code. A trap exits with status 134.
//
// As a library: createHost(opts) returns
//   { imports, instantiate(bytes, extraImports), instantiateModule(module, extraImports),
//     setMemory, close }.
// instantiate() handles modules that import a shared memory (threaded programs).
import fs from 'fs';
import os from 'os';
import { Worker } from 'worker_threads';

const READ_CHUNK = 1 << 16;

export const TRAP_NAMES = [
  'trap', 'index out of bounds', 'integer overflow', 'division by zero', 'int_cast out of range',
  'memory access out of bounds', 'length precondition failed', 'invalid union tag', 'stack exhausted',
  'invalid function pointer', 'misaligned atomic access', 'expectation failed',
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
    const base = info() >>> 0;
    const site = Number(dv.getBigUint64(base, true));
    if (site) {
      const cstr = (p) => {
        const u = new Uint8Array(memory.buffer);
        let e = p;
        while (u[e]) e++;
        return Buffer.from(u.subarray(p, e)).toString('utf8');
      };
      const wide = !!instance.exports.phalange_wasm64;
      const code = dv.getUint32(site, true);
      t = {
        code, name: TRAP_NAMES[code] || 'trap',
        fileId: dv.getUint32(site + 4, true), start: dv.getUint32(site + 8, true), end: dv.getUint32(site + 12, true),
        line: dv.getUint32(site + 16, true), col: dv.getUint32(site + 20, true),
        file: cstr(wide ? Number(dv.getBigUint64(site + 24, true)) : dv.getUint32(site + 24, true)),
        text: cstr(wide ? Number(dv.getBigUint64(site + 32, true)) : dv.getUint32(site + 28, true)),
      };
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
  return t;
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
// and sleeps in Atomics.wait until the worker has completed it.
class NetClient {
  constructor() {
    this.ctrl = new Int32Array(new SharedArrayBuffer(8));
    this.data = new Uint8Array(new SharedArrayBuffer(READ_CHUNK));
    this.worker = new Worker(new URL('./net_worker.mjs', import.meta.url), {
      workerData: { ctrl: this.ctrl.buffer, data: this.data.buffer },
    });
    this.worker.unref();
  }
  call(op, args, payload) {
    Atomics.store(this.ctrl, 0, 0);
    this.worker.postMessage({ op, args, payload });
    Atomics.wait(this.ctrl, 0, 0);
    return Atomics.load(this.ctrl, 1);
  }
  close() { this.worker.terminate(); }
}

export function createHost(opts = {}) {
  let memory = opts.memory || null;
  let module = null;
  let netClient = null;
  let nextThread = 1;
  const writeFd = opts.write || ((fd, bytes) => fs.writeSync(fd, bytes));
  const bytes = (p, n) => new Uint8Array(memory.buffer, p >>> 0, n >>> 0);
  const str = (p, n) => Buffer.from(memory.buffer, p >>> 0, n >>> 0).toString('utf8');
  const net = () => (netClient ||= new NetClient());
  const phalange = {
    ph_write(fd, p, n) {
      try { return writeFd(fd, bytes(p, n).slice()); } catch { return -1; }
    },
    ph_read(fd, p, n) {
      try {
        const buf = Buffer.alloc(Math.min(n >>> 0, READ_CHUNK));
        const got = fs.readSync(fd, buf, 0, buf.length, null);
        bytes(p, got).set(buf.subarray(0, got));
        return got;
      } catch (e) {
        return e.code === 'EOF' ? 0 : -1;
      }
    },
    ph_open(p, n, flags) {
      try { return fs.openSync(str(p, n), ['r', 'w', 'a'][flags] || 'r'); } catch (e) {
        return e.code === 'ENOENT' ? -2 : e.code === 'EACCES' ? -13 : -1;
      }
    },
    ph_close(fd) { try { fs.closeSync(fd); } catch { /* ignore */ } },
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
    ph_thread_spawn(entry, arg, stackTop) {
      const w = new Worker(new URL('./thread_worker.mjs', import.meta.url), {
        workerData: { module, memory, entry, arg, stackTop },
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

async function main() {
  const [file, entry] = process.argv.slice(2);
  if (!file) {
    console.error('usage: node runtime/host.mjs program.wasm [export]');
    process.exit(2);
  }
  const host = createHost();
  const instance = host.instantiate(fs.readFileSync(file));
  const fn = instance.exports[entry || (instance.exports.main ? 'main' : 'run')];
  if (typeof fn !== 'function') {
    console.error(`phalange: no export named '${entry || 'main'}'`);
    process.exit(2);
  }
  let code = 0;
  try {
    const r = fn();
    if (typeof r === 'number') code = r;
  } catch (e) {
    if (!(e instanceof WebAssembly.RuntimeError || e instanceof RangeError)) throw e;
    const t = describeTrap(instance, host.memory(), e);
    console.error(`phalange: ${trapMessage(t)}${t.file ? ` at ${t.file}:${t.line}:${t.col}` : ''}`);
    code = 134;
  }
  host.close();
  process.exit(code);
}

if (process.argv[1] && new URL(import.meta.url).pathname === fs.realpathSync(process.argv[1])) main();
