// runtime/thread_worker.mjs - runs one Phalange thread: a new instance of the
// program's module on the shared memory, entered through phalange_thread_start.
//
// A trap in any thread terminates the whole program (spec §12.8). The main
// thread may be blocked in Atomics.wait and unable to see worker events, so
// the trapping thread ends the process itself.
import fs from 'fs';
import { workerData } from 'worker_threads';
import { createHost, describeTrap, trapMessage } from './host.mjs';

const { module, memory, entry, arg, stackTop } = workerData;
const host = createHost({ memory });
let instance = null;
try {
  instance = host.instantiateModule(module);
  instance.exports.phalange_thread_start(entry, arg, stackTop);
} catch (e) {
  const t = describeTrap(instance, memory, e);
  fs.writeSync(2, `phalange: ${trapMessage(t)} in thread${t.file ? ` at ${t.file}:${t.line}:${t.col}` : ''}\n`);
  process.kill(process.pid, 'SIGABRT');
}
host.close();
