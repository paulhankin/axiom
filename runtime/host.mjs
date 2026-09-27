// runtime/host.mjs - the Phalange host ABI (import module "phalange") for
// WebAssembly under node, plus a runner:
//
//   node runtime/host.mjs program.wasm [export]
//
// calls `export` (default: main, else run). If it returns a number, that is
// the process exit code. A trap exits with status 134.
//
// As a library: createHost(opts) returns { imports, setMemory, close }.
import fs from 'fs';
import { Worker } from 'worker_threads';

const READ_CHUNK = 1 << 16;

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
  let memory = null;
  let netClient = null;
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
  };
  return {
    imports: { phalange },
    setMemory(m) { memory = m; },
    close() { if (netClient) netClient.close(); },
  };
}

async function main() {
  const [file, entry] = process.argv.slice(2);
  if (!file) {
    console.error('usage: node runtime/host.mjs program.wasm [export]');
    process.exit(2);
  }
  const host = createHost();
  const { instance } = await WebAssembly.instantiate(fs.readFileSync(file), host.imports);
  host.setMemory(instance.exports.memory);
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
