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
    console.error(`phalange: trap: ${e.message}`);
    code = 134;
  }
  host.close();
  process.exit(code);
}

if (process.argv[1] && new URL(import.meta.url).pathname === fs.realpathSync(process.argv[1])) main();
