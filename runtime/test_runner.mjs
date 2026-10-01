// runtime/test_runner.mjs - runs the tests of a Phalange test binary (wasm),
// each in a fresh instance in its own worker, several in parallel.
//
//   node runtime/test_runner.mjs tests.wasm [--filter=text] [--timeout=ms]
//
// Prints one line per test in the protocol read by `phalange test`:
//   PHT <index> <status> <code> <file_id> <start> <end> <ms> <message> <output> <stack>
// (<stack>: see test_main.c, encode_stack)
// (tab-separated; tabs, newlines and backslashes in fields are escaped).
import fs from 'fs';
import os from 'os';
import { BroadcastChannel, Worker } from 'worker_threads';

const args = process.argv.slice(2);
const file = args.find((a) => !a.startsWith('--'));
const filter = (args.find((a) => a.startsWith('--filter=')) || '--filter=').slice(9);
const timeout = Number((args.find((a) => a.startsWith('--timeout=')) || '--timeout=10000').slice(10));
const bytes = fs.readFileSync(file);
const module = new WebAssembly.Module(bytes);

// Names, read from a throwaway instance.
const names = await new Promise((resolve, reject) => {
  const w = new Worker(new URL('./test_worker.mjs', import.meta.url), { workerData: { bytes, list: true } });
  w.once('message', resolve);
  w.once('error', reject);
});

const esc = (s) => String(s ?? '').replace(/\\/g, '\\\\').replace(/\t/g, '\\t').replace(/\n/g, '\\n').replace(/\r/g, '\\r');
const selected = names.map((n, i) => i).filter((i) => !filter || names[i].includes(filter));
const results = new Map();
let next = 0;

function runOne(index) {
  return new Promise((resolve) => {
    const t0 = Date.now();
    // A trap in one of the test's threads arrives here (the test's own
    // thread may be blocked waiting for it).
    const trapChannel = `phalange-test-${process.pid}-${index}`;
    const bc = new BroadcastChannel(trapChannel);
    const w = new Worker(new URL('./test_worker.mjs', import.meta.url), { workerData: { bytes, index, trapChannel } });
    let done = false;
    const finish = (r) => {
      if (done) return;
      done = true;
      clearTimeout(timer);
      bc.close();
      r.ms = Date.now() - t0;
      results.set(index, r);
      w.terminate();
      resolve();
    };
    const timer = setTimeout(() => finish({ status: 'fail', code: 'timeout', message: `test did not finish within ${timeout} ms`, output: '' }), timeout);
    w.once('message', finish);
    bc.onmessage = (m) => finish(m.data);
    w.once('error', (e) => finish({ status: 'fail', code: 'T0000', message: `host error: ${e.message}`, output: '' }));
    w.once('exit', () => finish({ status: 'fail', code: 'T0000', message: 'test worker exited unexpectedly', output: '' }));
  });
}

async function pool() {
  while (next < selected.length) await runOne(selected[next++]);
}
const width = Math.max(1, Math.min(selected.length, os.availableParallelism ? os.availableParallelism() : os.cpus().length, 8));
await Promise.all(Array.from({ length: width }, pool));

for (const i of selected) {
  const r = results.get(i);
  process.stdout.write(['PHT', i, r.status, r.code || '', r.fileId ?? -1, r.start ?? 0, r.end ?? 0, r.ms,
    esc(r.message), esc(r.output), esc(r.stack)].join('\t') + '\n');
}
module; // (compiled once above to validate the file early)
