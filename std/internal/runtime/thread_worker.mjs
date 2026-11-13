// std/internal/runtime/thread_worker.mjs - runs one Axiom thread: a new instance of the
// program's module on the shared memory, entered through axiom_thread_start;
// or, with workerData.main, a threaded program's main thread, entered
// through its main (or run) export, under host.mjs's supervisor.
//
// A trap in any thread terminates the whole program (spec §12.8). The main
// thread may be blocked in Atomics.wait and unable to see worker events, so
// the trapping thread ends the process itself. Under `axiom test` it
// reports the trap as its test's result to the test runner instead, which
// ends that test.
import fs from 'fs';
import { BroadcastChannel, workerData } from 'worker_threads';
import { THREAD_EXIT, createHost, describeTrap, formatTrap, trapResult } from './host.mjs';

const { module, memory, entry, arg, stackTop, exited, netPort, shared, supervisor, testTrap, main } = workerData;
const host = createHost({ memory, netPort, shared, supervisor, thread: !main });
let instance = null;

// The main thread: its entry's result ends the program (through the
// supervisor, which first stops the profiler if there is one).
if (main) {
  const sup = { call: (op, args) => host.superCall(op, args) };
  instance = host.instantiateModule(module);
  const name = main.entry || (instance.exports.main ? 'main' : 'run');
  const fn = instance.exports[name];
  let code = 0;
  if (typeof fn !== 'function') {
    fs.writeSync(2, `axiom: no export named '${main.entry || 'main'}'\n`);
    code = 2;
  } else {
    if (main.profile) {
      const ss = instance.exports.axiom_shadow_stack;
      sup.call('profile_start', { ss: ss ? Number(ss()) : 0, wide: !!instance.exports.axiom_wasm64 });
    }
    try {
      const r = fn();
      if (typeof r === 'number') code = r;
    } catch (e) {
      if (!(e instanceof WebAssembly.RuntimeError || e instanceof RangeError || e instanceof WebAssembly.Exception)) throw e;
      fs.writeSync(2, formatTrap(describeTrap(instance, memory, e)) + '\n');
      code = 134;
    }
  }
  sup.call('main_done', { code, name });  // (does not return)
} else {
  run();
}

// Only once the entry has returned (or the thread ended itself: the host's
// ax_thread_exit) is the thread off its stack, which the program may then free.
function done() {
  const words = new Int32Array(memory.buffer);
  const i = Number(exited) / 4;
  Atomics.store(words, i, 1);
  Atomics.notify(words, i, 1);
}

function run() {
  try {
    instance = host.instantiateModule(module);
    try {
      // (its three pointers are i64s on wasm64: BigInts; the host took them as Numbers)
      const p = instance.exports.axiom_wasm64 ? BigInt : Number;
      instance.exports.axiom_thread_start(p(entry), p(arg), p(stackTop));
    } catch (e) {
      if (e !== THREAD_EXIT) throw e;
    }
    done();
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
}
