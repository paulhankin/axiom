// runtime/test_worker.mjs - runs one test of a test binary in a fresh
// instance; see test_runner.mjs.
import fs from 'fs';
import { parentPort, workerData } from 'worker_threads';
import { createHost, describeTrap, trapMessage, readStack, stackFrames } from './host.mjs';

const { bytes, index, list } = workerData;
let output = '';
const dec = new TextDecoder();
const host = createHost({
  write: (fd, b) => {
    if (fd === 1 || fd === 2) { output += dec.decode(b); return b.length; }
    return fs.writeSync(fd, b);
  },
});
const instance = host.instantiate(bytes);
const mem = () => host.memory();
const cstr = (p) => {
  const u = new Uint8Array(mem().buffer);
  let e = p;
  while (u[e]) e++;
  return Buffer.from(u.subarray(p, e)).toString('utf8');
};
const ex = instance.exports;

// Frames in the PHT protocol (see test_main.c: encode_stack).
function encodeFrames(frames, kindOfFirst) {
  return frames.map((f, i) => {
    if (f.repeat) return `R\x1f${f.repeat}\x1f0\x1f0\x1f`;
    if (f.omitted) return `O\x1f${f.omitted}\x1f0\x1f0\x1f`;
    const k = i === 0 ? kindOfFirst : 'C';
    return `${f.file ? k : 'U'}\x1f${f.fileId ?? 0}\x1f${f.start ?? 0}\x1f${f.end ?? 0}\x1f${f.fn || ''}`;
  }).map((x) => x + '\x1e').join('');
}

if (list) {
  const n = ex.phalange_test_count();
  const names = [];
  for (let i = 0; i < n; i++) names.push(cstr(ex.phalange_test_name(i) >>> 0));
  parentPort.postMessage(names);
} else {
  const expect = ex.phalange_test_expect(index);
  let r;
  try {
    const code = ex.phalange_test_run(index) >>> 0;
    if (code !== 0) {
      r = { status: 'fail', code: 'error', message: `test returned error.${cstr(ex.phalange_error_name(code) >>> 0)}` };
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
    const t = describeTrap(instance, mem(), e);
    if (expect === -2 || expect === t.code) {
      r = { status: 'pass' };
    } else {
      r = { status: 'fail', code: t.codeName, message: trapMessage(t), fileId: t.fileId, start: t.start, end: t.end,
        stack: encodeFrames(stackFrames(t, t.stack), 'T') };
      if (expect >= 0) r.message = `expected trap T${String(expect).padStart(4, '0')}, got ${r.message}`;
    }
  }
  r.output = output;
  host.close();
  parentPort.postMessage(r);
}
