// runtime/thread_worker.mjs - runs one Axiom thread: a new instance of the
// program's module on the shared memory, entered through axiom_thread_start.
//
// A trap in any thread terminates the whole program (spec §12.8). The main
// thread may be blocked in Atomics.wait and unable to see worker events, so
// the trapping thread ends the process itself. Under `axiom test` it
// reports the trap as its test's result to the test runner instead, which
// ends that test.
import fs from 'fs';
import { BroadcastChannel, workerData } from 'worker_threads';
import { createHost, describeTrap, formatTrap, trapResult } from './host.mjs';

const { module, memory, entry, arg, stackTop, exited, netPort, shared, testTrap } = workerData;
const host = createHost({ memory, netPort, shared });
let instance = null;
try {
  instance = host.instantiateModule(module);
  instance.exports.axiom_thread_start(entry, arg, stackTop);
  // Only now is the thread off its stack, which the program may then free.
  const words = new Int32Array(memory.buffer);
  const i = Number(exited) / 4;
  Atomics.store(words, i, 1);
  Atomics.notify(words, i, 1);
} catch (e) {
  const t = describeTrap(instance, memory, e);
  if (testTrap) {
    const r = trapResult(t, testTrap.expect);
    r.output = '';
    const bc = new BroadcastChannel(testTrap.channel);
    bc.postMessage(r);
    bc.close();
    host.close();
    process.exit(0);  // ends this thread only; the runner ends the test
  }
  fs.writeSync(2, formatTrap(t, ' in thread') + '\n');
  process.kill(process.pid, 'SIGABRT');
}
host.close();
