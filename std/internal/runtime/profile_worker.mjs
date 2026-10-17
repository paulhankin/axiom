// std/internal/runtime/profile_worker.mjs - samples an Axiom program's shadow stack
// (spec §16.4, §16.6) from another thread. The program runs with a shared
// memory, so this worker can read the main thread's call-site ring while the
// program runs. Samples are wall-clock. See host.mjs --profile.
import { parentPort, workerData } from 'worker_threads';

const { memory, ss, wide, hz, control } = workerData;
const ctl = new Int32Array(control);
const SS_CAP = 256;
const samples = new Map();
const period = 1000 / hz;

let buf = null, u32 = null, u64 = null;
function views() {
  if (buf !== memory.buffer) {
    buf = memory.buffer;
    u32 = new Uint32Array(buf);
    u64 = wide ? new BigUint64Array(buf) : null;
  }
}

parentPort.postMessage('ready');
while (Atomics.load(ctl, 0) === 0) {
  views();
  const depth = Atomics.load(u32, ss >>> 2);
  const n = Math.min(depth, SS_CAP);
  const frames = new Array(n);
  for (let i = 0; i < n; i++) {
    const slot = ss + 8 + ((depth - 1 - i) & (SS_CAP - 1)) * (wide ? 8 : 4);
    frames[i] = wide ? Number(u64[slot >>> 3]) : u32[slot >>> 2];
  }
  const key = depth + ',' + frames.join(',');
  samples.set(key, (samples.get(key) || 0) + 1);
  Atomics.wait(ctl, 1, 0, period);
}
parentPort.postMessage([...samples]);
