// runtime/test_worker.mjs - runs one test of a test binary in a fresh
// instance; see test_runner.mjs.
import fs from 'fs';
import { parentPort, workerData } from 'worker_threads';
import { createHost, describeTrap, readStack, trapResult } from './host.mjs';

const { bytes, index, list, trapChannel } = workerData;
let output = '';
const dec = new TextDecoder();
const hostOpts = {
  write: (fd, b) => {
    if (fd === 1 || fd === 2) { output += dec.decode(b); return b.length; }
    return fs.writeSync(fd, b);
  },
};
const host = createHost(hostOpts);
const instance = host.instantiate(bytes);
const mem = () => host.memory();
const cstr = (p) => {
  const u = new Uint8Array(mem().buffer);
  let e = p;
  while (u[e]) e++;
  return Buffer.from(u.subarray(p, e)).toString('utf8');
};
const ex = instance.exports;

if (list) {
  const n = ex.axiom_test_count();
  const names = [];
  for (let i = 0; i < n; i++) names.push(cstr(ex.axiom_test_name(i) >>> 0));
  parentPort.postMessage(names);
} else {
  const expect = ex.axiom_test_expect(index);
  // a trap in one of the test's threads is reported by that thread
  hostOpts.testTrap = { channel: trapChannel, expect };
  let r;
  try {
    const code = ex.axiom_test_run(index) >>> 0;
    if (code !== 0) {
      r = { status: 'fail', code: 'error', message: `test returned error.${cstr(ex.axiom_error_name(code) >>> 0)}` };
      const st = readStack(instance, mem());
      if (st) r.stack = st.errors.map((e) => `E\x1f${e.fileId}\x1f${e.start}\x1f${e.end}\x1f${e.fn}\x1e`).join('');
    } else if (expect !== -1) {
      r = { status: 'fail', code: 'no_trap', message: expect === -2 ? 'expected a trap, but the test completed'
        : `expected trap T${String(expect).padStart(4, '0')}, but the test completed` };
    } else {
      r = { status: 'pass' };
    }
  } catch (e) {
    if (!(e instanceof WebAssembly.RuntimeError || e instanceof RangeError)) throw e;
    r = trapResult(describeTrap(instance, mem(), e), expect);
  }
  r.output = output;
  host.close();
  parentPort.postMessage(r);
}
