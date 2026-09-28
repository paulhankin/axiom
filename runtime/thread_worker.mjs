// runtime/thread_worker.mjs - runs one Phalange thread: a new instance of the
// program's module on the shared memory, entered through phalange_thread_start.
//
// A trap in any thread terminates the whole program (spec §12.8). The main
// thread may be blocked in Atomics.wait and unable to see worker events, so
// the trapping thread ends the process itself.
import fs from 'fs';
import { workerData } from 'worker_threads';
import { createHost, describeTrap, formatTrap } from './host.mjs';

const { module, memory, entry, arg, stackTop, exited, netPort } = workerData;
const host = createHost({ memory, netPort });
let instance = null;
try {
  instance = host.instantiateModule(module);
  instance.exports.phalange_thread_start(entry, arg, stackTop);
  // Only now is the thread off its stack, which the program may then free.
  const words = new Int32Array(memory.buffer);
  const i = Number(exited) / 4;
  Atomics.store(words, i, 1);
  Atomics.notify(words, i, 1);
} catch (e) {
  const t = describeTrap(instance, memory, e);
  fs.writeSync(2, formatTrap(t, ' in thread') + '\n');
  process.kill(process.pid, 'SIGABRT');
}
host.close();
