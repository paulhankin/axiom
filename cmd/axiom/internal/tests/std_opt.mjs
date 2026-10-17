// cmd/axiom/internal/tests/std_opt.mjs - the standard library's tests at -O2, on wasm32 and native.
//
// `node cmd/axiom/internal/tests/run.mjs` runs each std module's `axiom test` once, at the
// optimization level given (default -O0) and on one target. This sweep builds
// and runs every module's tests optimized on both targets, in parallel, with
// the compiler's own consistency checks on:
//   AXIOM_CHECK_CINTS=1  shared constants are never changed by a pass
//   AXIOM_POISON=1       released memory is poisoned and never reused
// (both checked by the self-hosted compiler; the C one ignores them).
//
// Usage: node cmd/axiom/internal/tests/std_opt.mjs [-O1|-O2|-O3] [--target=wasm32|native|both]
//                               [--jobs=N] [--filter=substr] [--timeout=seconds]
//                               [--sanitize=address] [--test-timeout=ms]
//
// The compiler is $AXIOM, else cmd/axiom/build/axiom when built, else
// cmd/axiom/bootstrap/build/axiom. AXIOM_NATIVE_TARGET and AXIOM_NATIVE_EXEC work as in
// cmd/axiom/internal/tests/run.mjs.
import { spawn } from 'child_process';
import fs from 'fs';
import path from 'path';
import os from 'os';

const root = path.resolve(path.dirname(new URL(import.meta.url).pathname), '../../../..');
const selfhost = path.join(root, 'cmd/axiom/build/axiom');
const compiler = path.resolve(process.env.AXIOM || (fs.existsSync(selfhost) ? selfhost : path.join(root, 'cmd/axiom/bootstrap/build/axiom')));
const argv = process.argv.slice(2);
const opt = argv.find((a) => /^-O[0-3]$/.test(a)) || '-O2';
const arg = (name, dflt) => (argv.find((a) => a.startsWith(`--${name}=`)) || `=${dflt}`).split('=').slice(1).join('=');
const filter = arg('filter', '');
const jobs = Math.max(1, Number(arg('jobs', os.availableParallelism ? os.availableParallelism() : os.cpus().length)));
const timeoutMs = Number(arg('timeout', 900)) * 1000;
const which = arg('target', 'both');
const extra = argv.filter((a) => a.startsWith('--sanitize='));
// each test's own limit (axiom test --timeout; default 10 s, 60 s under emulation)
const testTimeout = argv.filter((a) => a.startsWith('--test-timeout=')).map((a) => `--timeout=${a.slice(15)}`);
const execWrap = process.env.AXIOM_NATIVE_EXEC || '';
const targets = [];
if (which === 'wasm32' || which === 'both') targets.push({ name: 'wasm32', flags: [] });
if (which === 'native' || which === 'both') {
  // Emulated tests run several times slower: give each test a minute.
  targets.push({ name: process.env.AXIOM_NATIVE_TARGET || 'native',
    flags: [`--target=${process.env.AXIOM_NATIVE_TARGET || 'native'}`, ...(execWrap ? ['--timeout=60000'] : [])] });
}
if (!targets.length) { console.error(`unknown --target=${which}`); process.exit(2); }

function files(dir) {
  return fs.readdirSync(dir).filter((f) => f.endsWith('.ax')).map((f) => path.join(dir, f)).sort();
}
function dirsUnder(dir) {
  return [dir, ...fs.readdirSync(dir, { withFileTypes: true })
    .filter((e) => e.isDirectory() && e.name !== 'testdata').map((e) => e.name).sort()
    .flatMap((n) => dirsUnder(path.join(dir, n)))];
}
const flagsOf = (src) => [...src.matchAll(/^\/\/ flags: (.*)$/gm)].flatMap((m) => m[1].trim().split(/\s+/));

const work = [];
for (const f of dirsUnder(path.join(root, 'std')).flatMap(files)) {
  const src = fs.readFileSync(f, 'utf8');
  if (!/^test "/m.test(src)) continue;
  for (const t of targets) {
    const name = `${path.relative(root, f)} [${[t.name, opt, ...extra].join(' ')}]`;
    if (filter && !name.includes(filter)) continue;
    work.push({ name, args: ['test', f, '--format=agent-json', opt, ...extra, ...t.flags, ...testTimeout, ...flagsOf(src)] });
  }
}

const env = { ...process.env, AXIOM_CHECK_CINTS: '1', AXIOM_POISON: '1' };

function run(w) {
  return new Promise((resolve) => {
    const t0 = Date.now();
    const p = spawn(compiler, w.args, { cwd: root, env, stdio: ['ignore', 'pipe', 'pipe'] });
    let out = '', err = '', timedOut = false;
    p.stdout.on('data', (d) => { out += d; });
    p.stderr.on('data', (d) => { err += d; });
    const timer = setTimeout(() => { timedOut = true; p.kill('SIGKILL'); }, timeoutMs);
    p.on('close', (status, signal) => {
      clearTimeout(timer);
      const ms = Date.now() - t0;
      if (timedOut) return resolve({ ms, problem: `timed out after ${timeoutMs / 1000} s` });
      let json = null;
      try { json = JSON.parse(out); } catch { /* reported below */ }
      if (!json) return resolve({ ms, problem: `bad output (status ${status}${signal ? `, ${signal}` : ''}):\n${out}${err}` });
      if (json.status !== 'pass') {
        const failed = json.tests ? json.tests.filter((t) => t.status !== 'pass') : json;
        return resolve({ ms, problem: `failures:\n${JSON.stringify(failed, null, 1)}` });
      }
      resolve({ ms, problem: null });
    });
  });
}

let pass = 0, fail = 0, next = 0;
const failures = [];
const t0 = Date.now();
await Promise.all(Array.from({ length: Math.min(jobs, work.length) }, async () => {
  while (next < work.length) {
    const w = work[next++];
    const r = await run(w);
    if (r.problem) { fail++; failures.push(`FAIL ${w.name}: ${r.problem}`); console.log(`FAIL ${w.name}`); }
    else { pass++; console.log(`ok   ${w.name} (${r.ms} ms)`); }
  }
}));
for (const f of failures) console.log('\n' + f);
console.log(`\n${pass} passed, ${fail} failed (${path.relative(root, compiler)}, ${jobs} jobs, ${((Date.now() - t0) / 1000).toFixed(0)} s)`);
process.exit(fail ? 1 : 0);
