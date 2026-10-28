// cmd/axiom/internal/tests/run.mjs - Axiom test runner.
//
// Directives (in comments anywhere in a test file):
//   // expect: <line>          expected output line (in order)
//   // expect-trap             `run` must trap
//   // expect-error: E0412     compilation must fail with this code (repeatable)
//   // expect-warning: W0710   this warning on this line; with any, the file's
//                              warnings must be exactly those declared
//   // flags: --overflow=trap  extra compiler flags
//   // wasm-only               skip on the native target
//   // native-only             skip on wasm
//   // selfhost-only           skip with the bootstrap compiler (LLVM's optimizer
//                              differs, e.g. it folds an integer round trip of a pointer)
//   // x86-64-flags: --cpu=x86-64  extra flags for a native build on an x86-64 host
//   // link: file.c            C source (next to the test) compiled and linked in
//   // header: name.h          generate a C header for the exports (for the C source)
//
// Usage: node cmd/axiom/internal/tests/run.mjs [--native] [--filter=substr] [-O2] [--units=module|program] [-g]
//                                            [--list] [--verbose]
//
// --list prints the names of the tests that would run; --verbose names the
// tests skipped and why. Environment: AXIOM_TEST_JOBS (tests run at a time;
// default the CPUs, 1 is serial); AXIOM_TEST_SKIP_PLATFORM=1 skips what does
// not depend on the target and flags (fmt, macOS, Windows, the wasm32 check
// of cmd/axiom/src), for runs after the first of several configurations.
//
// The suite is a list of sections (section(...) below): a map from test
// names to functions that return null, a problem (a string), or
// { skip: true, reason }. Sections run their tests one at a time, or `jobs`
// at a time with `parallel`; a section whose `supported` check fails is
// skipped whole, and `note` marks results that were built but not run.
import { spawn, spawnSync } from 'child_process';
import fs from 'fs';
import path from 'path';
import zlib from 'zlib';
import os from 'os';
import { Worker } from 'worker_threads';
import { createHost } from '../../../../std/internal/runtime/host.mjs';
import { checkMachO } from './macho.mjs';

const root = path.resolve(path.dirname(new URL(import.meta.url).pathname), '../../../..');
const compiler = path.resolve(process.env.AXIOM || path.join(root, 'cmd/axiom/bootstrap/build/axiom'));
const argv = process.argv.slice(2);
const native = argv.includes('--native');
// Cross testing: AXIOM_NATIVE_TARGET=aarch64 AXIOM_NATIVE_EXEC="qemu-aarch64 -L /usr/aarch64-linux-gnu"
const nativeTarget = `--target=${process.env.AXIOM_NATIVE_TARGET || 'native'}`;
// Windows (AXIOM_NATIVE_TARGET=x86_64-windows AXIOM_NATIVE_EXEC=wine64) and
// macOS link no C code: cmd/axiom/internal/tests/native_rt.ax replaces cmd/axiom/internal/tests/native_rt.c, and
// tests that call C are skipped.
const noC = /windows|macos|darwin/.test(process.env.AXIOM_NATIVE_TARGET || '');
const execWrap = (process.env.AXIOM_NATIVE_EXEC || '').split(' ').filter((x) => x);
const spawnExe = (exe, args, opts) => execWrap.length ? spawnSync(execWrap[0], [...execWrap.slice(1), exe, ...args], opts) : spawnSync(exe, args, opts);
const filter = (argv.find(a => a.startsWith('--filter=')) || '').slice(9);
const listOnly = argv.includes('--list');
const verbose = argv.includes('--verbose');
const extraFlags = argv.filter(a => /^-O[0-3]$/.test(a) || /^--units=(module|program)$/.test(a) || a === '-g');
const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'axiom-tests-'));
// the temporary directory goes whatever ends the run: the summary, an
// uncaught error, or Ctrl-C
process.on('exit', () => fs.rmSync(tmp, { recursive: true, force: true }));
process.on('SIGINT', () => process.exit(130));
process.on('SIGTERM', () => process.exit(143));
// No build or run may hang the suite: a compile gets five minutes, a program
// two (more when emulated), and an `axiom test` run a minute per test.
const compileTimeout = 300000;
const runTimeout = (process.env.AXIOM_NATIVE_EXEC ? 4 : 1) * 120000;
const jobs = Math.max(1, Number(process.env.AXIOM_TEST_JOBS) || os.availableParallelism());
const skipPlatform = !!process.env.AXIOM_TEST_SKIP_PLATFORM && process.env.AXIOM_TEST_SKIP_PLATFORM !== '0';
// The compiler checks that the optimized IR's instructions come after their
// operands (cmd/axiom/README.md; the bootstrap compiler ignores it).
process.env.AXIOM_CHECK_IR ??= '1';
// Emulated tests run several times slower, and tests sharing the CPUs
// slower too: give each test a minute.
const testTimeout = ['--timeout=60000'];

// spawnSync's result ({ status, signal, stdout, stderr, error }; text), as a
// promise. Honors cwd, env and timeout.
function spawnAsync(cmd, args, opts = {}) {
  const { timeout, encoding, maxBuffer, ...rest } = opts;
  return new Promise((resolve) => {
    const out = [], err = [];
    let error, timer, done = false;
    const finish = (status, signal) => {
      if (done) return;
      done = true;
      clearTimeout(timer);
      resolve({ status, signal, stdout: Buffer.concat(out).toString('utf8'), stderr: Buffer.concat(err).toString('utf8'), error });
    };
    let p;
    try { p = spawn(cmd, args, { ...rest, stdio: ['ignore', 'pipe', 'pipe'] }); } catch (e) { error = e; finish(null, null); return; }
    p.stdout.on('data', (b) => out.push(b));
    p.stderr.on('data', (b) => err.push(b));
    p.on('error', (e) => { error = e; if (p.pid === undefined) finish(null, null); });
    p.on('close', finish);
    if (timeout) timer = setTimeout(() => { error = Object.assign(new Error('timed out'), { code: 'ETIMEDOUT' }); p.kill('SIGTERM'); }, timeout);
  });
}
const spawnExeAsync = (exe, args, opts) => execWrap.length ? spawnAsync(execWrap[0], [...execWrap.slice(1), exe, ...args], opts) : spawnAsync(exe, args, opts);

function files(dir) {
  return fs.readdirSync(dir).filter(f => f.endsWith('.ax')).map(f => path.join(dir, f)).sort();
}

function directives(src) {
  const d = { expect: [], trap: false, errors: [], warnings: [], flags: [], x64Flags: [], wasmOnly: false, nativeOnly: false, selfhostOnly: false, links: [], header: null };
  for (const [i, line] of src.split('\n').entries()) {
    let m;
    if ((m = line.match(/\/\/ expect: ?(.*)$/))) d.expect.push(m[1]);
    else if ((m = line.match(/\/\/ expect-warning: (W\d+)/))) d.warnings.push(`${m[1]} at line ${i + 1}`);
    else if (/\/\/ expect-trap\b/.test(line)) d.trap = true;
    else if ((m = line.match(/\/\/ expect-error: (E\d+)/))) d.errors.push(m[1]);
    else if ((m = line.match(/\/\/ flags: (.*)$/))) d.flags.push(...m[1].trim().split(/\s+/));
    else if ((m = line.match(/\/\/ x86-64-flags: (.*)$/))) d.x64Flags.push(...m[1].trim().split(/\s+/));
    else if (/\/\/ wasm-only\b/.test(line)) d.wasmOnly = true;
    else if (/\/\/ native-only\b/.test(line)) d.nativeOnly = true;
    else if (/\/\/ selfhost-only\b/.test(line)) d.selfhostOnly = true;
    else if ((m = line.match(/\/\/ link: (\S+)/))) d.links.push(m[1]);
    else if ((m = line.match(/\/\/ header: (\S+)/))) d.header = m[1];
  }
  return d;
}

async function runWasm(file) {
  // Output from the test print helpers (env) and from fd 1 (std/io) is
  // collected as one text stream, in order; fd 2's is kept for the report.
  let text = '', errText = '';
  let mem;
  const dec = new TextDecoder();
  const line = (s) => { text += s + '\n'; };
  const host = createHost({
    write: (fd, bytes) => {
      if (fd === 1) {
        text += dec.decode(bytes);
        return bytes.length;
      }
      if (fd === 2) {
        errText += dec.decode(bytes);
        return bytes.length;
      }
      return fs.writeSync(fd, bytes);
    },
  });
  const env = {
    print_i64: x => line(String(x)),
    print_u64: x => line(String(BigInt.asUintN(64, x))),
    print_f64: x => line(String(x)),
    print_str: (p, n) => line(dec.decode(new Uint8Array(mem.buffer, p >>> 0, n >>> 0))),
    print_bool: b => line(b ? 'true' : 'false'),
  };
  const instance = host.instantiate(fs.readFileSync(file), { env });
  mem = instance.exports.memory || host.memory();
  const lines = () => { const o = text.split('\n'); if (o[o.length - 1] === '') o.pop(); return o; };
  try {
    (instance.exports.run || instance.exports.main)();
    return { out: lines(), trapped: false, stderr: errText };
  } catch (e) {
    if (e instanceof WebAssembly.RuntimeError || e instanceof RangeError) return { out: lines(), trapped: true, msg: e.message, stderr: errText };
    throw e;
  } finally {
    host.close();
  }
}

async function runNative(exe) {
  const r = await spawnExeAsync(exe, [], { encoding: 'utf8', timeout: runTimeout });
  if (r.error && r.error.code === 'ETIMEDOUT') return { out: [], trapped: true, msg: `timed out after ${runTimeout / 1000} s` };
  const out = r.stdout.split('\n');
  if (out[out.length - 1] === '') out.pop();
  return { out, trapped: r.status !== 0, msg: r.signal || `exit ${r.status}` };
}

console.log(`compiler ${path.relative(root, compiler)}, ${native ? `native (${nativeTarget || 'this machine'}${execWrap.length ? `, run with ${execWrap.join(' ')}` : ''})` : 'wasm32'}${extraFlags.length ? `, ${extraFlags.join(' ')}` : ''}, ${jobs} jobs`);
let pass = 0, fail = 0, skip = 0;
const failures = [], skips = [];

// One result: r is null (passed), a string (the problem), { skip: true,
// reason }, or { problem, stderr } (stderr is shown, as the program wrote it).
function record(name, r, ms, note) {
  if (r && r.skip) {
    skip++;
    skips.push(`${name}${r.reason ? `: ${r.reason}` : ''}`);
    if (verbose) console.log(`skip ${name}${r.reason ? ` (${r.reason})` : ''}`);
    return;
  }
  if (r && typeof r === 'object') {
    if (r.stderr) process.stderr.write(r.stderr);
    r = r.problem;
  }
  if (r) { fail++; failures.push(`FAIL ${name}: ${r}`); console.log(`FAIL ${name}`); }
  else { pass++; console.log(`ok   ${name}${ms === undefined ? '' : ` (${ms.toFixed(0)} ms)`}${note ? ` (${note})` : ''}`); }
}

// A section of the suite. `cases` maps names (after `prefix: `, when there
// is one) to functions, sync or async. Options: parallel (run `jobs` at a
// time, reporting in order); supported (a function: null, or the reason the
// whole section is skipped, called only when a case passes the filter, so
// an excluded section costs nothing); note (added to each ok line, e.g.
// "built, not run: no Wine"); timed (false: no duration on the ok lines).
async function section(prefix, cases, opts = {}) {
  const entries = Object.entries(cases).map(([n, fn]) => [prefix ? `${prefix}: ${n}` : n, fn])
    .filter(([n]) => !filter || n.includes(filter));
  if (!entries.length) return;
  if (listOnly) { for (const [n] of entries) console.log(n); return; }
  const unsupported = opts.supported ? await opts.supported() : null;
  if (unsupported) { for (const [n] of entries) record(n, { skip: true, reason: unsupported }); return; }
  const timed = opts.timed !== false;
  const run = async ([name, fn]) => {
    const t0 = process.hrtime.bigint();
    let r;
    try { r = await fn(); } catch (e) { r = `threw: ${e && e.stack ? e.stack : e}`; }
    return { name, r, ms: timed ? Number(process.hrtime.bigint() - t0) / 1e6 : undefined };
  };
  const show = ({ name, r, ms }) => record(name, r, ms, opts.note);
  if (!opts.parallel) {
    for (const e of entries) show(await run(e));
    return;
  }
  // `jobs` at a time, reported in order
  const results = new Array(entries.length), done = new Array(entries.length).fill(false);
  let next = 0, shown = 0;
  const worker = async () => {
    while (next < entries.length) {
      const i = next++;
      results[i] = await run(entries[i]);
      done[i] = true;
      while (shown < entries.length && done[shown]) show(results[shown++]);
    }
  };
  await Promise.all(Array.from({ length: Math.min(jobs, entries.length) }, worker));
}
// A map of cases from files: name (relative to the repository, with a tag)
// to fn(file).
const byFile = (list, tag, fn) => Object.fromEntries(list.map((f) => [`${path.relative(root, f)}${tag ? ` ${tag}` : ''}`, () => fn(f)]));
const hostRun = (out) => (native ? spawnExe(out, [], { encoding: 'utf8' })
  : spawnSync(process.execPath, [path.join(root, 'std/internal/runtime/host.mjs'), out], { encoding: 'utf8' }));
const targetFlags = native ? [nativeTarget] : [];
let isBootstrapCompiler = null;
const bootstrapCompiler = () => {
  if (isBootstrapCompiler === null) isBootstrapCompiler = /bootstrap/.test(spawnSync(compiler, ['version'], { encoding: 'utf8' }).stdout);
  return isBootstrapCompiler;
};

// Run-and-compare programs and expected compile errors. Each test builds in
// a directory of its own (run/ and errors/ share names).
await section('', byFile([...files(path.join(root, 'cmd/axiom/internal/tests/run')), ...files(path.join(root, 'cmd/axiom/internal/tests/errors'))], '', async (f) => {
  const src = fs.readFileSync(f, 'utf8');
  const d = directives(src);
  if ((native && d.wasmOnly) || (!native && d.nativeOnly)) return { skip: true, reason: native ? 'wasm only' : 'native only' };
  if (d.selfhostOnly && bootstrapCompiler()) return { skip: true, reason: "the bootstrap compiler's optimizer differs" };
  if (native && noC && (d.links.length || /extern "C"/.test(src))) return { skip: true, reason: 'links C code' };
  const dir = path.join(tmp, path.basename(path.dirname(f)) + '-' + path.basename(f, '.ax'));
  fs.mkdirSync(dir);
  const out = path.join(dir, path.basename(f, '.ax') + (native ? '' : '.wasm'));
  const args = ['build', f, '--format=agent-json', '-o', out, ...d.flags, ...extraFlags];
  if (native) args.push(nativeTarget, `--link=${path.join(root, noC ? 'cmd/axiom/internal/tests/native_rt.ax' : 'cmd/axiom/internal/tests/native_rt.c')}`);
  if (native && process.arch === 'x64' && !process.env.AXIOM_NATIVE_TARGET) args.push(...d.x64Flags);
  for (const l of d.links) args.push(`--link=${path.join(path.dirname(f), l)}`);
  if (d.header) args.push(`--emit-header=${path.join(dir, d.header)}`);
  const c = await spawnAsync(compiler, args, { encoding: 'utf8', timeout: compileTimeout });
  let json = null;
  try { json = JSON.parse(c.stdout); } catch { json = null; }
  if (c.error && c.error.code === 'ETIMEDOUT') return `the compiler did not finish in ${compileTimeout / 1000} s`;
  if (d.errors.length) {
    const codes = json ? json.diagnostics.filter(x => x.severity === 'error').map(x => x.code) : [];
    if (c.status === 0) return `expected errors ${d.errors.join(', ')}, but compilation succeeded`;
    for (const e of d.errors) if (!codes.includes(e)) return `expected ${e}, got [${codes.join(', ')}]:\n${c.stdout}${c.stderr}`;
    return null;
  }
  if (c.status !== 0) return `compilation failed:\n${c.stdout}${c.stderr}`;
  if (d.warnings.length) {
    const got = (json ? json.diagnostics : []).filter((x) => x.severity === 'warning' && path.resolve(x.file) === f)
      .map((x) => `${x.code} at line ${x.span.start_line}`);
    if (got.sort().join(', ') !== [...d.warnings].sort().join(', ')) return `expected warnings [${d.warnings.join(', ')}], got [${got.join(', ')}]`;
  }
  const r = native ? await runNative(out) : await runWasm(out);
  let problem = null;
  if (d.trap && !r.trapped) problem = 'expected a trap, but run completed';
  if (!d.trap && r.trapped) problem = `unexpected trap: ${r.msg}`;
  const want = d.expect.join('\n'), got = r.out.join('\n');
  if (!problem && want !== got) problem = `output mismatch\n--- expected\n${want}\n--- got\n${got}`;
  return { problem, stderr: r.stderr };
}), { parallel: true });

// `axiom test`: the standard library's own tests must pass, and the
// self-tests in cmd/axiom/internal/tests/testing/ must produce exactly the declared results.
const testTargets = native ? [nativeTarget] : [];
function dirsUnder(dir) {
  return [dir, ...fs.readdirSync(dir, { withFileTypes: true })
    .filter((e) => e.isDirectory() && e.name !== 'testdata').map((e) => e.name).sort()
    .flatMap((n) => dirsUnder(path.join(dir, n)))];
}
const stdDirs = dirsUnder(path.join(root, 'std'));
const stdFiles = stdDirs.flatMap(files).filter((f) => /^test "/m.test(fs.readFileSync(f, 'utf8')));
await section('', byFile([...stdFiles, ...files(path.join(root, 'cmd/axiom/internal/tests/testing'))], '[axiom test]', async (f) => {
  const src = fs.readFileSync(f, 'utf8');
  const d = directives(src);
  const c = await spawnAsync(compiler, ['test', f, '--format=agent-json', ...testTargets, ...testTimeout, ...d.flags, ...extraFlags], { encoding: 'utf8' });
  let json = null;
  try { json = JSON.parse(c.stdout); } catch { return `bad output (status ${c.status}):\n${c.stdout}${c.stderr}`; }
  const want = [...src.matchAll(/^\/\/ test-result: (.+?) \| (pass|fail)(?: \| (\S+))?(?: \| (\d+))?$/gm)];
  if (!want.length) {
    return json.status !== 'pass' ? `failures:\n${JSON.stringify(json.tests ? json.tests.filter((t) => t.status !== 'pass') : json, null, 1)}` : null;
  }
  for (const [, tname, status, code, line] of want) {
    const t = json.tests.find((x) => x.name === tname);
    if (!t) return `missing result for "${tname}"`;
    if (t.status !== status) return `"${tname}": expected ${status}, got ${t.status} ${JSON.stringify(t.failure)}`;
    if (code && t.failure.code !== code) return `"${tname}": expected code ${code}, got ${t.failure.code}`;
    if (line && t.failure.span.start_line !== Number(line)) return `"${tname}": expected line ${line}, got ${t.failure.span.start_line}`;
  }
  return json.tests.length !== want.length ? `expected ${want.length} tests, got ${json.tests.length}` : null;
}), { parallel: true });

// Stack traces (spec §16.4): `// stack:` and `// errors:` lines in
// cmd/axiom/internal/tests/traces/traces.ax give the expected frames of each failing test, and
// cmd/axiom/internal/tests/traces/crash.ax must report a stack overflow with its stack.
await section('', {
  'cmd/axiom/internal/tests/traces/traces.ax [stack traces]': () => {
    const f = path.join(root, 'cmd/axiom/internal/tests/traces/traces.ax');
    const src = fs.readFileSync(f, 'utf8');
    const c = spawnSync(compiler, ['test', f, '--format=agent-json', ...testTargets, ...testTimeout, ...extraFlags], { encoding: 'utf8' });
    let json = null;
    try { json = JSON.parse(c.stdout); } catch { return `bad output (status ${c.status}):\n${c.stdout}${c.stderr}`; }
    if (!json.tests) return `no test results (status ${c.status}):\n${c.stdout}${c.stderr}`;
    const render = (frames) => frames.map((x) => (x.repeat ? `~${x.repeat}` : x.omitted ? null : x.line ? `${x.function}@${x.line}` : x.function))
      .filter((x) => x !== null).join(' ');
    for (const [, kind, tname, want] of src.matchAll(/^\/\/ (stack|errors): (.+?) \| (.+)$/gm)) {
      const t = json.tests.find((x) => x.name === tname);
      const frames = t && t.failure && (kind === 'stack' ? t.failure.stack : t.failure.error_trace);
      const got = frames ? render(frames) : '(none)';
      if (got !== want.trim()) return `"${tname}" ${kind}:\n  expected ${want.trim()}\n  got      ${got}`;
    }
    return null;
  },
  'cmd/axiom/internal/tests/traces/crash.ax [crash report]': () => {
    const out = path.join(tmp, 'crash' + (native ? '' : '.wasm'));
    const b = spawnSync(compiler, ['build', path.join(root, 'cmd/axiom/internal/tests/traces/crash.ax'), '-o', out, ...targetFlags, ...extraFlags], { encoding: 'utf8' });
    if (b.status !== 0) return `build failed: ${b.stdout}${b.stderr}`;
    // (a timeout: a recursion turned into a loop never overflows)
    const r = native ? spawnExe(out, [], { encoding: 'utf8', timeout: 60000 })
      : spawnSync(process.execPath, [path.join(root, 'std/internal/runtime/host.mjs'), out], { encoding: 'utf8', timeout: 60000 });
    const want = [/trap T0008 \(stack exhausted\)/, /stack trace \(innermost first\):/, /at forever \((?:\S*\/)?tests\/traces\/crash\.ax:4:12\)/,
      /the frame above repeats 255 more times/, /\.\.\. \d+ more frames/];
    for (const w of want) if (!w.test(r.stderr)) return `stderr lacks ${w}:\n${r.stderr}`;
    return r.status === 0 ? 'the program exited normally' : null;
  },
}, { timed: false });

// The address sanitizer (spec §16.8): the programs in cmd/axiom/internal/tests/sanitize/ are
// built with --sanitize=address. Each states its exit status ("// status: N",
// or "trap" for T0012) and lines its stderr must contain ("// stderr: ...");
// "// status: 0" programs must not report anything.
await section('', byFile(files(path.join(root, 'cmd/axiom/internal/tests/sanitize')), '[--sanitize=address]', async (f) => {
  const src = fs.readFileSync(f, 'utf8');
  const st = (src.match(/^\/\/ status: (\w+)/m) || [null, '0'])[1];
  const wants = [...src.matchAll(/^\/\/ stderr: (.+)$/gm)].map((m) => m[1].trim());
  const out = path.join(tmp, 'asan-' + path.basename(f, '.ax') + (native ? '' : '.wasm'));
  const b = await spawnAsync(compiler, ['build', f, '--sanitize=address', '-o', out, ...targetFlags, ...extraFlags], { encoding: 'utf8' });
  if (b.status !== 0) return `build failed: ${b.stdout}${b.stderr}`;
  const r = native ? await spawnExeAsync(out, [], { encoding: 'utf8' })
    : await spawnAsync(process.execPath, [path.join(root, 'std/internal/runtime/host.mjs'), out], { encoding: 'utf8' });
  if (st === 'trap') {
    if (r.status === 0 || !/trap T0012 \(memory error\)/.test(r.stderr)) return `expected trap T0012, got status ${r.status}:\n${r.stderr}`;
  } else if (r.status !== Number(st)) {
    return `expected status ${st}, got ${r.status} ${r.signal || ''}:\n${r.stderr}`;
  }
  if (st === '0' && /AddressSanitizer/.test(r.stderr)) return `unexpected report:\n${r.stderr}`;
  for (const w of wants) if (!r.stderr.includes(w)) return `stderr lacks "${w}":\n${r.stderr}`;
  return null;
}), { parallel: true, timed: false });

// Instruction selection: each "// lower: <flags> | <regex> | <count>" line
// of cmd/axiom/internal/tests/lowering/*.ax builds the file with --emit=asm and those flags,
// and the regex must match exactly <count> times. x86-64 lines run only on
// x86-64 hosts (--target=native is the host). A compiler without
// --emit=asm (the compiler) is checked on its object file as
// objdump disassembles it (AT&T syntax, as LLVM's): its x86-64 lines, where
// objdump is installed; it skips the others. "// lower-selfhost:" lines
// check selection only the compiler's backends make (LLVM's differs).
await section('', byFile(files(path.join(root, 'cmd/axiom/internal/tests/lowering')), '[instruction selection]', async (f) => {
  let checked = 0, reason = 'no line applies to this host';
  for (const [, only, flags, re, count] of fs.readFileSync(f, 'utf8').matchAll(/^\/\/ lower(-selfhost)?: (.+?) \| (.+?) \| (\d+)$/gm)) {
    const fl = flags.trim().split(/\s+/);
    if (fl.includes('--target=native') && (process.arch !== 'x64' || process.env.AXIOM_NATIVE_TARGET)) continue;
    const out = path.join(tmp, `lower-${path.basename(f, '.ax')}.s`);
    const c = await spawnAsync(compiler, ['build', f, '--emit=asm', '-o', out, ...fl], { encoding: 'utf8' });
    let text = null;
    if (c.status !== 0 && /--emit=asm is not supported/.test(c.stderr + c.stdout)) {
      if (!fl.includes('--target=native')) { reason = 'the compiler has no --emit=asm for this target'; continue; }
      const obj = path.join(tmp, `lower-${path.basename(f, '.ax')}.o`);
      const o = await spawnAsync(compiler, ['build', f, '--emit=obj', '-o', obj, ...fl], { encoding: 'utf8' });
      if (o.status !== 0) return `${flags}: build failed:\n${o.stdout}${o.stderr}`;
      const d = await spawnAsync('objdump', ['-d', '--no-show-raw-insn', obj]);
      if (d.error || d.status !== 0) { reason = 'no objdump'; continue; }
      text = d.stdout;
    } else if (c.status !== 0) {
      return `${flags}: build failed:\n${c.stdout}${c.stderr}`;
    } else if (only) {
      reason = "the bootstrap compiler's selection differs";
      continue;  // (a compiler with --emit=asm: the bootstrap compiler's LLVM)
    } else {
      text = fs.readFileSync(out, 'utf8');
    }
    checked++;
    const n = (text.match(new RegExp(re, 'g')) || []).length;
    if (n !== Number(count)) return `${flags}: /${re}/ matches ${n} times, expected ${count}`;
  }
  return checked === 0 ? { skip: true, reason } : null;
}), { parallel: true, timed: false });

// Profiling (spec §16.6).
await section('', {
  'cmd/axiom/internal/tests/profile/hot.ax [axiom profile]': () => {
    const folded = path.join(tmp, 'hot.folded');
    const c = spawnSync(compiler, ['profile', path.join(root, 'cmd/axiom/internal/tests/profile/hot.ax'), '--format=agent-json', `--folded=${folded}`,
      ...testTargets, ...extraFlags, '--', 'fast', 'loop'], { encoding: 'utf8' });
    let j = null;
    try { j = JSON.parse(c.stdout); } catch { return `bad output (status ${c.status}):\n${c.stdout}${c.stderr}`; }
    const fn = (n) => j.functions.find((f) => f.name === n);
    const site = (a, b) => j.call_sites.find((x) => x.caller === a && x.callee === b);
    if (j.schema !== 'axiom-profile/1' || j.samples < 20) return `too few samples: ${c.stdout}`;
    if (!fn('spin') || fn('spin').self_pct < 50) return `spin is not the hot function: ${JSON.stringify(j.functions)}`;
    if (!fn('main') || fn('main').total_pct < 90) return `main should be on (almost) every stack: ${JSON.stringify(j.functions)}`;
    if (!site('work', 'spin') || site('work', 'spin').line !== 21) return `missing call site work -> spin: ${JSON.stringify(j.call_sites)}`;
    if (!/^main;work;spin \d+$/m.test(fs.readFileSync(folded, 'utf8'))) return `folded stacks lack main;work;spin:\n${fs.readFileSync(folded, 'utf8')}`;
    return null;
  },
}, { timed: false });

// Separate compilation (spec §16.7): the object cache is reused and
// invalidated along imports, cached objects stay correct when the program's
// error codes are renumbered, and the cache is kept under $AXIOM_CACHE_LIMIT
// (the least recently used objects go, but nothing used or written within
// the last hour); `axiom cache size` reports the cache.
const unitsCopy = (name) => {
  const dir = path.join(tmp, name);
  fs.mkdirSync(dir, { recursive: true });
  for (const f of ['lib.ax', 'main.ax']) fs.copyFileSync(path.join(root, 'cmd/axiom/internal/tests/units', f), path.join(dir, f));
  return dir;
};
await section('cmd/axiom/internal/tests/units', {
  '[separate compilation]': () => {
    const dir = unitsCopy('units');
    const env = { ...process.env, AXIOM_CACHE: path.join(tmp, 'units-cache') };
    const out = path.join(tmp, 'units-prog' + (native ? '' : '.wasm'));
    let want = ['42', '3', '9', 'too big', 'TooBig', '5'].join('\n');
    const lib = path.join(dir, 'lib.ax');
    const replace = (from, to) => {
      const t = fs.readFileSync(lib, 'utf8');
      if (!t.includes(from)) throw new Error(`units test: lib.ax lacks ${from}`);
      fs.writeFileSync(lib, t.replace(from, to));
    };
    // each step: an edit, the modules it should recompile (of t), the output
    const steps = [
      ['cold build', () => {}, (n, t) => n === t],
      ['no change', () => {}, (n) => n === 0],
      ['new error name renumbers all codes', () => fs.appendFileSync(path.join(dir, 'main.ax'), '\nerror { Aardvark }\n'), (n) => n === 1],
      ['library edit outside a body', () => fs.appendFileSync(lib, '\n// edited\n'), (n) => n === 2],
      // importers depend on a module's interface: the body of a function
      // that is neither generic nor inline is the module's own business
      ['library function body edit', () => replace('    calls += 1;\n', '    calls += 1;\n    _ = xs.len;\n'), (n) => n === 1],
      ['library function body edit seen in the output', () => {
        replace('return "too big";', 'return "too large";');
        want = want.replace('too big', 'too large');
      }, (n) => n === 1],
      ['generic method body edit', () => replace('        return self.b;\n', '        _ = self.a;\n        return self.b;\n'), (n) => n === 2],
      ['library signature edit', () => fs.appendFileSync(lib, '\npub fn more() u32 {\n    return 1;\n}\n'), (n) => n === 2],
      // on wasm the thread-locals of the whole program share one block whose
      // layout every module's code uses, so a new one recompiles everything
      ['new thread-local', () => fs.appendFileSync(lib, '\nthreadlocal let mut more2: u8 = 0;\n'),
        (n, t) => native ? n === 2 : n === t],
    ];
    for (const [what, edit, ok] of steps) {
      edit();
      const c = spawnSync(compiler, ['build', path.join(dir, 'main.ax'), '-o', out, '--verbose', '--units=module', ...targetFlags], { encoding: 'utf8', env });
      const m = /(\d+) modules, (\d+) compiled/.exec(c.stderr);
      if (c.status !== 0 || !m) return `${what}: build failed:\n${c.stdout}${c.stderr}`;
      if (!ok(Number(m[2]), Number(m[1]))) return `${what}: ${m[0]}`;
      const r = hostRun(out);
      if (r.stdout.trim() !== want) return `${what}: output\n${r.stdout}${r.stderr}`;
    }
    return null;
  },
  '[cache eviction]': () => {
    const dir = unitsCopy('evict');
    const cache = path.join(tmp, 'evict-cache');
    const env = { ...process.env, AXIOM_CACHE: cache, AXIOM_CACHE_LIMIT: '1' };
    const out = path.join(tmp, 'evict-prog' + (native ? '' : '.wasm'));
    const build = () => spawnSync(compiler, ['build', path.join(dir, 'main.ax'), '-o', out, '--verbose', '--units=module', ...targetFlags], { encoding: 'utf8', env });
    const objs = () => fs.readdirSync(path.join(cache, 'obj')).sort();
    const c1 = build();
    const m = /(\d+) modules, (\d+) compiled/.exec(c1.stderr);
    if (c1.status !== 0 || !m) return `cold build failed:\n${c1.stdout}${c1.stderr}`;
    if (/evicted/.test(c1.stderr)) return `fresh objects evicted: ${c1.stderr}`;
    const fresh = objs();
    // stale files (objects of old builds, a temporary of an interrupted one)
    // are over the limit and go; the objects this build uses stay
    const old = (Date.now() - 3 * 3600 * 1000) / 1000;
    for (const f of ['stale1.o', 'stale2.o', 'x.o.tmp999']) {
      const p = path.join(cache, 'obj', f);
      fs.writeFileSync(p, Buffer.alloc(4096));
      fs.utimesSync(p, old, old);
    }
    const size = spawnSync(compiler, ['cache', 'size'], { encoding: 'utf8', env });
    if (!new RegExp(`^${cache.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')}/obj: ${fresh.length + 3} objects, .* \\(limit 1 B\\)\n$`).test(size.stdout))
      return `cache size: ${size.stdout}${size.stderr}`;
    const c2 = build();
    if (c2.status !== 0) return `second build failed:\n${c2.stdout}${c2.stderr}`;
    if (!/(\d+) modules, 0 compiled/.test(c2.stderr)) return `second build recompiled: ${c2.stderr}`;
    if (!/evicted 3 cached objects \(12\.0 KiB\)/.test(c2.stderr)) return `eviction not reported: ${c2.stderr}`;
    return objs().join() !== fresh.join() ? `objects after eviction: ${objs().join(' ')}` : null;
  },
}, { timed: false });

// The node host's directory handles (ax_dir_*) pin the directory they
// opened: a symbolic link swapped in for it afterwards is not followed,
// so fs.remove_all and fs.walk cannot be led out of their tree.
await section('std/internal/runtime/host.mjs', {
  'directory handles do not follow a swapped-in link': () => {
    const dir = path.join(tmp, 'dirswap');
    fs.mkdirSync(path.join(dir, 'a/sub'), { recursive: true });
    fs.writeFileSync(path.join(dir, 'a/sub/f'), 'x');
    fs.mkdirSync(path.join(dir, 'victim'));
    fs.writeFileSync(path.join(dir, 'victim/keep'), 'x');
    const script = `
      import fs from 'node:fs';
      import { createHost } from ${JSON.stringify(path.join(root, 'std/internal/runtime/host.mjs'))};
      const dir = ${JSON.stringify(dir)};
      const memory = new WebAssembly.Memory({ initial: 1 });
      const ax = createHost({ memory }).axiom;
      const put = (s) => { const b = Buffer.from(s); new Uint8Array(memory.buffer, 0, b.length).set(b); return [0, b.length]; };
      const da = ax.ax_dir_open(-1, ...put(dir + '/a'));
      const ds = ax.ax_dir_open(da, ...put('sub'));
      fs.renameSync(dir + '/a/sub', dir + '/a/sub.old');
      fs.symlinkSync(dir + '/victim', dir + '/a/sub');
      const n = ax.ax_dir_read(ds, 1000, 100);
      const listing = n > 0 ? Buffer.from(new Uint8Array(memory.buffer, 1000, n)).toString('latin1') : String(n);
      const removed = ax.ax_dir_remove(ds, ...put('keep'), 0);
      const opened = ax.ax_dir_open(da, ...put('sub'));
      console.log(JSON.stringify({ da, ds, listing, removed, opened }));
    `;
    const r = spawnSync(process.execPath, ['--input-type=module', '-e', script], { encoding: 'utf8' });
    let o;
    try { o = JSON.parse(r.stdout); } catch (e) { return `${e.message}\n${r.stdout}${r.stderr}`; }
    if (o.da < 0 || o.ds < 0) return `could not open: ${r.stdout}`;
    // through the pinned descriptor (Linux) the handle still lists the old
    // directory; elsewhere it notices the swap (ELOOP, -40)
    if (o.listing !== '\u0001f\u0000' && o.listing !== '-40') return `listing after the swap: ${JSON.stringify(o.listing)}`;
    if (o.removed !== -2 && o.removed !== -40) return `remove through the handle: ${o.removed}`;
    if (o.opened !== -40) return `opening the link: ${o.opened}`;
    return fs.existsSync(path.join(dir, 'victim/keep')) ? null : 'victim/keep was removed';
  },
}, { timed: false });

// axiom test --show-output (spec §17.2): a passing test's output is shown
// only when asked for.
await section('axiom test', {
  '--show-output': () => {
    const dir = path.join(tmp, 'showout');
    fs.mkdirSync(dir, { recursive: true });
    const src = path.join(dir, 't.ax');
    fs.writeFileSync(src, [
      'import "std/io" as io;', '',
      'test "prints" {',
      '    io.stdout().write_all("hello from the test\\n")!;',
      '}', ''].join('\n'));
    const quiet = spawnSync(compiler, ['test', src, ...targetFlags], { encoding: 'utf8' });
    const shown = spawnSync(compiler, ['test', src, '--show-output', ...targetFlags], { encoding: 'utf8' });
    if (quiet.status !== 0 || shown.status !== 0) return `tests failed:\n${quiet.stdout}${quiet.stderr}${shown.stdout}${shown.stderr}`;
    if (/output:/.test(quiet.stdout)) return `output shown without the flag:\n${quiet.stdout}`;
    return /ok   prints \(\d+ ms\)\n     output:\n       \| hello from the test\n/.test(shown.stdout) ? null : `output not shown:\n${shown.stdout}`;
  },
}, { timed: false });

// The tour (docs/tour.md): each program in docs/tour/ appears verbatim in
// the page, builds for the suite's target, and prints its .out file;
// tests.ax's tests pass.
{
  const tourDir = path.join(root, 'docs/tour');
  const page = () => fs.readFileSync(path.join(root, 'docs/tour.md'), 'utf8');
  await section('', byFile(files(tourDir), '', (f) => {
    const base = path.basename(f, '.ax');
    const src = fs.readFileSync(f, 'utf8');
    if (!page().includes('```axiom\n' + src.trimEnd() + '\n```')) return 'the program is not in docs/tour.md as written';
    const dir = path.join(tmp, 'tour-' + base);
    fs.mkdirSync(dir, { recursive: true });
    const outFile = path.join(tourDir, base + '.out');
    if (!fs.existsSync(outFile)) {
      const c = spawnSync(compiler, ['test', f, ...targetFlags], { encoding: 'utf8' });
      return c.status !== 0 || !/\n3 passed, 0 failed/.test(c.stdout) ? `tests:\n${c.stdout}${c.stderr}` : null;
    }
    const out = path.join(dir, base + (native ? '' : '.wasm'));
    const c = spawnSync(compiler, ['build', f, '-o', out, ...targetFlags], { encoding: 'utf8' });
    if (c.status !== 0) return `build failed:\n${c.stdout}${c.stderr}`;
    const r = hostRun(out);
    const want = fs.readFileSync(outFile, 'utf8');
    if (r.status !== 0) return `exit ${r.status}:\n${r.stdout}${r.stderr}`;
    return r.stdout !== want ? `output:\n${r.stdout}--- expected:\n${want}` : null;
  }), { timed: false });
}

// Diagnostic codes (spec §16.3): the table lists exactly the codes the two
// compilers emit, and every emitted code is asserted by some test. A code
// that no test can reach is listed here with the reason.
await section('spec §16.3', {
  'diagnostic codes, both compilers and the tests agree': () => {
    const codesIn = (text) => new Set([...text.matchAll(/"(E\d{4})"/g)].map((m) => m[1]));
    const readAll = (dir, ext) => fs.readdirSync(dir).filter((f) => f.endsWith(ext)).map((f) => fs.readFileSync(path.join(dir, f), 'utf8')).join('\n');
    const emittedAx = codesIn(readAll(path.join(root, 'cmd/axiom/src'), '.ax'));
    const emittedC = codesIn(readAll(path.join(root, 'cmd/axiom/bootstrap/src'), '.c'));
    const spec = fs.readFileSync(path.join(root, 'docs/spec.md'), 'utf8');
    const tableText = spec.slice(spec.indexOf('### 16.3 Diagnostic codes'), spec.indexOf('### 16.4 '));
    const table = new Set([...tableText.matchAll(/^\| (E\d{4}) \|/gm)].map((m) => m[1]));
    // codes a test asserts: expect-error directives, and codes named in the suite's own checks
    const asserted = new Set();
    const walk = (dir) => {
      for (const e of fs.readdirSync(dir, { withFileTypes: true })) {
        const p = path.join(dir, e.name);
        if (e.isDirectory()) walk(p);
        else if (/\.(ax|mjs|toml)$/.test(e.name)) for (const m of fs.readFileSync(p, 'utf8').matchAll(/\bE\d{4}\b/g)) asserted.add(m[0]);
      }
    };
    walk(path.join(root, 'cmd/axiom/internal/tests'));
    const unreachable = {
      E0651: 'needs 65535 error names',
      E9003: 'a tool the command needs is missing (node, cc)',
      E9004: 'the system linker failed',
      E9006: 'the test runner itself failed',
      E9007: 'an internal failure of code generation or formatting',
    };
    const problems = [];
    const diff = (a, b) => [...a].filter((x) => !b.has(x)).sort();
    const onlyAx = diff(emittedAx, emittedC), onlyC = diff(emittedC, emittedAx);
    if (onlyAx.length) problems.push(`emitted only by the compiler: ${onlyAx.join(' ')}`);
    if (onlyC.length) problems.push(`emitted only by the bootstrap compiler: ${onlyC.join(' ')}`);
    const notInTable = diff(emittedAx, table), notEmitted = diff(table, emittedAx);
    if (notInTable.length) problems.push(`emitted but not in the spec's table: ${notInTable.join(' ')}`);
    if (notEmitted.length) problems.push(`in the spec's table but emitted by no compiler: ${notEmitted.join(' ')}`);
    const untested = diff(emittedAx, asserted).filter((c) => !(c in unreachable));
    if (untested.length) problems.push(`emitted but asserted by no test: ${untested.join(' ')}`);
    return problems.length ? `\n  ${problems.join('\n  ')}` : null;
  },
}, { timed: false });

// Flags with no other assertion: each builds a program that then runs, or
// produces the text asked for; `axiom version`; the version module's tests.
{
  const dir = path.join(tmp, 'flags');
  const src = path.join(dir, 'p.ax');
  const prepare = () => {
    fs.mkdirSync(dir, { recursive: true });
    fs.writeFileSync(src, 'import "std/io" as io;\n\nexport fn main() i32 {\n    io.stdout().write_all("flag ok\\n") catch return 1;\n    return 0;\n}\n');
  };
  const isBootstrap = () => /bootstrap/.test(spawnSync(compiler, ['version'], { encoding: 'utf8' }).stdout);
  const runs = (out) => {
    const r = hostRun(out);
    return r.status === 0 && r.stdout === 'flag ok\n' ? null : `ran wrong: ${r.status}\n${r.stdout}${r.stderr}`;
  };
  const builds = (flag, n) => () => {
    prepare();
    const o = path.join(dir, n + (native ? '' : '.wasm'));
    const c = spawnSync(compiler, ['build', src, flag, '-o', o, ...targetFlags], { encoding: 'utf8' });
    return c.status ? c.stderr : runs(o);
  };
  await section('flags', {
    '--stack-traces=off': builds('--stack-traces=off', 'st'),
    '--overflow=wrap': builds('--overflow=wrap', 'ow'),
    '--units=program': builds('--units=program', 'up'),
    '-g': builds('-g', 'g'),
    '--emit=asm (the bootstrap compiler) or --emit=obj': () => {
      if (!native) return { skip: true, reason: 'native only' };
      prepare();
      // the compiler's native backends write objects, not assembly
      const asm = isBootstrap();
      const o = path.join(dir, asm ? 'p.s' : 'p.o');
      const c = spawnSync(compiler, ['build', src, asm ? '--emit=asm' : '--emit=obj', '-o', o, nativeTarget], { encoding: 'utf8' });
      if (c.status) return c.stderr;
      if (asm) {
        const text = fs.readFileSync(o, 'utf8');
        return /main/.test(text) && text.length > 100 ? null : `assembly without main:\n${text.slice(0, 200)}`;
      }
      return fs.statSync(o).size > 100 ? null : 'an empty object file';
    },
    'an --emit=obj object links into a C program (examples/c_interop)': () => {
      if (!native || isBootstrap() || process.platform !== 'linux') return { skip: true, reason: 'the compiler on Linux only' };
      if (spawnSync('cc', ['--version']).status !== 0) return { skip: true, reason: 'no C compiler' };
      const ex = path.join(root, 'examples/c_interop');
      const o = path.join(dir, 'stats.o');
      const h = path.join(dir, 'stats.h');
      const c = spawnSync(compiler, ['build', path.join(ex, 'stats.ax'), '--emit=obj', `--emit-header=${h}`, '-o', o,
        nativeTarget], { encoding: 'utf8' });
      if (c.status) return c.stderr;
      const exe = path.join(dir, 'c_interop');
      const l = spawnSync('cc', [`-I${dir}`, path.join(ex, 'main.c'), o, '-o', exe], { encoding: 'utf8' });
      if (l.status) return `cc: ${l.stdout}${l.stderr}`;
      const r = spawnSync(exe, [], { encoding: 'utf8' });
      return r.status === 0 && /sorted 5 values/.test(r.stdout) ? null : `status ${r.status}: ${r.stdout}${r.stderr}`;
    },
    '--emit=ir': () => {
      if (isBootstrap()) return { skip: true, reason: 'the bootstrap compiler has --emit=llvm instead' };
      prepare();
      const o = path.join(dir, 'p.ir');
      const c = spawnSync(compiler, ['build', src, '--emit=ir', '-o', o, ...targetFlags], { encoding: 'utf8' });
      if (c.status) return c.stderr;
      const text = fs.readFileSync(o, 'utf8');
      return /main/.test(text) && /global/.test(text) ? null : `IR without main:\n${text.slice(0, 200)}`;
    },
    'axiom version': () => {
      const v = /pub let VERSION: \[\]u8 = "([^"]+)"/.exec(fs.readFileSync(path.join(root, 'cmd/axiom/src/version.ax'), 'utf8'))[1];
      const c = spawnSync(compiler, ['version'], { encoding: 'utf8' });
      return c.status === 0 && c.stdout.startsWith(`axiom ${v}`) ? null : `version: ${c.stdout}${c.stderr}`;
    },
    'cmd/axiom/src/version.ax tests': () => {
      const c = spawnSync(compiler, ['test', path.join(root, 'cmd/axiom/src/version.ax'), ...targetFlags], { encoding: 'utf8' });
      return c.status === 0 && /\n\d+ passed, 0 failed/.test(c.stdout) ? null : `${c.stdout}${c.stderr}`;
    },
    'unknown target is E9002': () => {
      prepare();
      const c = spawnSync(compiler, ['build', src, '--target=vax', '--format=agent-json', '-o', path.join(dir, 'x')], { encoding: 'utf8' });
      return c.status === 2 && /"E9002"/.test(c.stdout) ? null : `${c.status}: ${c.stdout}${c.stderr}`;
    },
    'axiom new over an existing directory is E9005': () => {
      prepare();
      const c = spawnSync(compiler, ['new', 'flags', '--format=agent-json'], { encoding: 'utf8', cwd: tmp });
      return c.status !== 0 && /"E9005"/.test(c.stdout) ? null : `${c.status}: ${c.stdout}${c.stderr}`;
    },
  }, { timed: false });
}

// The compiler reaches the system only through the host ABI, so
// it must stay buildable for WebAssembly: no libc, no 64-bit
// usize assumptions.
if (!skipPlatform) {
  await section('', {
    'cmd/axiom/src checks for wasm32': () => {
      const c = spawnSync(compiler, ['check', path.join(root, 'cmd/axiom/src/main.ax'), '--target=wasm32'], { encoding: 'utf8' });
      return c.status !== 0 ? `\n${c.stdout}${c.stderr}` : null;
    },
  }, { timed: false });
}

// A copy of the standard library under the working directory (as when
// testing a checkout of std without --std) is not the library: with the
// address sanitizer, which links the library's std/mem, its modules must not
// clash with the library's in separate compilation.
await section('', {
  'std copy under the working directory [--sanitize=address]': () => {
    const dir = path.join(tmp, 'stdcopy');
    fs.mkdirSync(path.join(dir, 'std'), { recursive: true });
    fs.copyFileSync(path.join(root, 'std/mem.ax'), path.join(dir, 'std/mem.ax'));
    fs.writeFileSync(path.join(dir, 'std/app.ax'), [
      'import "mem.ax" as mem;', '',
      'export fn main() i32 {',
      '    let b = mem.alloc.[u8](100) catch return 1;',
      '    b[99] = 7;',
      '    let v = b[99];',
      '    mem.free(b);',
      '    return @as(i32, v) - 7;',
      '}', ''].join('\n'));
    const out = path.join(tmp, 'stdcopy-prog' + (native ? '' : '.wasm'));
    const c = spawnSync(compiler, ['build', 'std/app.ax', '-o', out, '--sanitize=address', '--units=module', ...targetFlags],
      { encoding: 'utf8', cwd: dir, env: { ...process.env, AXIOM_CACHE: path.join(tmp, 'stdcopy-cache') } });
    if (c.status !== 0) return `build failed:\n${c.stdout}${c.stderr}`;
    const r = hostRun(out);
    return r.status !== 0 ? `exit status ${r.status}:\n${r.stderr}` : null;
  },
}, { timed: false });

// `axiom fmt` (spec §16.5; the compiler only, not the bootstrap one). Each
// cmd/axiom/internal/tests/fmt/<name>.ax formats to <name>.expected, which formats to itself;
// --check lists what is not canonical; a file with a syntax error is left
// alone; the repository's own sources are canonical.
if (!skipPlatform) {
  const fdir = path.join(root, 'cmd/axiom/internal/tests/fmt');
  const fmt = (args) => spawnSync(compiler, ['fmt', ...args], { encoding: 'utf8', cwd: root });
  const cases = {};
  for (const f of files(fdir)) {
    const want = f.replace(/\.ax$/, '.expected');
    cases[path.basename(f)] = () => {
      const r = fmt(['--stdout', f]);
      if (r.status !== 0) return `status ${r.status}: ${r.stderr}`;
      if (r.stdout !== fs.readFileSync(want, 'utf8')) return `output differs from ${path.basename(want)}:\n${r.stdout}`;
      const again = fmt(['--check', want]);
      return again.status === 0 ? null : `${path.basename(want)} is not stable: ${again.stdout}${again.stderr}`;
    };
  }
  cases['--check and rewriting'] = () => {
    const f = path.join(tmp, 'messy.ax');
    fs.writeFileSync(f, 'fn  f( ) i32 { return 1 ; }\n');
    const c = fmt(['--check', f]);
    if (c.status !== 1 || c.stdout.trim() !== f) return `--check: status ${c.status}, output ${c.stdout}`;
    const w = fmt([f]);
    if (w.status !== 0) return `rewrite: status ${w.status}: ${w.stderr}`;
    const text = fs.readFileSync(f, 'utf8');
    return text === 'fn f() i32 {\n    return 1;\n}\n' ? null : `rewritten as:\n${text}`;
  };
  cases['syntax errors are reported'] = () => {
    const f = path.join(tmp, 'broken.ax');
    const src = 'fn f() i32 { return 1 }\n';
    fs.writeFileSync(f, src);
    const r = fmt(['--format=agent-json', f]);
    if (r.status !== 1 || !/E0100/.test(r.stdout)) return `status ${r.status}: ${r.stdout}${r.stderr}`;
    return fs.readFileSync(f, 'utf8') === src ? null : 'the file was changed';
  };
  cases['the repository is canonical'] = () => {
    // everything but tests/errors and tests/fmt, whose files are not canonical on purpose
    const tests = fs.readdirSync(path.join(root, 'cmd/axiom/internal/tests'))
      .filter((n) => n !== 'errors' && n !== 'fmt' && (n.endsWith('.ax') ||
        fs.statSync(path.join(root, 'cmd/axiom/internal/tests', n)).isDirectory()))
      .map((n) => `cmd/axiom/internal/tests/${n}`);
    const r = fmt(['--check', 'cmd/axiom/src', 'cmd/axiom/bench', 'std', 'examples', 'docs/tour', ...tests]);
    return r.status === 0 ? null : `not canonical:\n${r.stdout}${r.stderr}`;
  };
  await section('fmt', cases, {
    supported: () => (/usage:/.test(fmt(['--check', path.join(fdir, 'layout.expected')]).stderr) ? 'the bootstrap compiler has no axiom fmt' : null),
    timed: false,
  });
}

// macOS (spec §14.1; the compiler only, not the bootstrap one). cmd/axiom/internal/tests/macho.mjs checks
// the executables built for --target=aarch64-macos the way dyld and the
// kernel would; the compiler itself is among them. On a Mac they also run
// (and the whole suite runs natively with AXIOM_NATIVE_TARGET=aarch64-macos);
// elsewhere `axiom test` for the target says it needs a Mac.
if (!skipPlatform) {
  const onMac = process.platform === 'darwin' && os.arch() === 'arm64';
  const out = (n) => path.join(tmp, 'macos-' + n);
  const build = (src, n, extra = []) => spawnSync(compiler, ['build', src, '--target=aarch64-macos', '-o', out(n), ...extra],
    { encoding: 'utf8', cwd: root, env: { ...process.env, AXIOM_CACHE: path.join(tmp, 'macos-cache') } });
  const check = (src, n, extra) => {
    const c = build(src, n, extra);
    if (c.status !== 0) return `build failed:\n${c.stdout}${c.stderr}`;
    const r = checkMachO(fs.readFileSync(out(n)));
    if (r.errs.length) return r.errs.join('\n');
    return (fs.statSync(out(n)).mode & 0o111) ? null : 'not executable';
  };
  await section('macos', {
    'thread-locals and output': () => {
      const src = path.join(tmp, 'macos-hello.ax');
      fs.writeFileSync(src, 'import "std/io" as io;\n\nthreadlocal let mut tl: u64 = 41;\n\nexport fn main() i32 {\n' +
        '    tl += 1;\n    io.stdout().println("hello") catch return 1;\n    return @int_cast(i32, tl) - 42;\n}\n');
      const p = check(src, 'hello', ['-O2']);
      if (p || !onMac) return p;
      const r = spawnSync(out('hello'), [], { encoding: 'utf8' });
      return r.status === 0 && r.stdout === 'hello\n' ? null : `status ${r.status}: ${r.stdout}${r.stderr}`;
    },
    'examples': () => {
      for (const e of ['wordcount', 'http_server', 'texmath']) {
        const p = check(path.join(root, `examples/${e}/${e}.ax`), e);
        if (p) return `${e}: ${p}`;
      }
      return null;
    },
    'the compiler': () => {
      const p = check(path.join(root, 'cmd/axiom/src/main.ax'), 'axiom', ['-O1']);
      if (p || !onMac) return p;
      // the Mac-built compiler builds a program that runs
      const src = path.join(tmp, 'macos-hello.ax');
      const c = spawnSync(out('axiom'), ['build', src, '--target=native', `--std=${path.join(root, 'std')}`, '-o', out('hello2')],
        { encoding: 'utf8', cwd: root, env: { ...process.env, AXIOM_CACHE: path.join(tmp, 'macos-cache2') } });
      if (c.status !== 0) return `the Mac compiler failed: ${c.stdout}${c.stderr}`;
      const r = spawnSync(out('hello2'), [], { encoding: 'utf8' });
      return r.status === 0 && r.stdout === 'hello\n' ? null : `status ${r.status}: ${r.stdout}${r.stderr}`;
    },
    'frameworks: Objective-C, Metal, Quartz, GameController, Core Audio (cmd/axiom/internal/tests/macos)': () => {
      const tests = fs.readdirSync(path.join(root, 'cmd/axiom/internal/tests/macos')).filter((f) => f.endsWith('.ax')).sort()
        .map((f) => `cmd/axiom/internal/tests/macos/${f}`);
      for (const f of tests) {
        const c = spawnSync(compiler, ['check', f, '--target=aarch64-macos'], { encoding: 'utf8', cwd: root });
        if (c.status !== 0) return `${f}: check failed:\n${c.stdout}${c.stderr}`;
        if (!onMac) continue;
        const r = spawnSync(compiler, ['test', f, '--target=aarch64-macos'], { encoding: 'utf8', cwd: root,
          env: { ...process.env, AXIOM_CACHE: path.join(tmp, 'macos-cache') } });
        if (r.status !== 0 || !/passed, 0 failed/.test(r.stdout)) return `${f}: status ${r.status}: ${r.stdout}${r.stderr}`;
      }
      return null;
    },
    'examples on Metal: metal, game, trench, swarm': () => {
      for (const e of ['metal', 'game', 'trench', 'swarm']) {
        const p = check(path.join(root, `examples/${e}/${e}.ax`), e, ['-O2']);
        if (p) return `${e}: ${p}`;
        if (!onMac) continue;
        // a frame offscreen (the games play a while by themselves first)
        const png = out(`${e}.png`);
        const r = spawnSync(out(e), ['--png', png, ...(e === 'trench' || e === 'swarm' ? ['--frames', '120'] : [])],
          { encoding: 'utf8', cwd: root, timeout: 120000 });
        if (r.status !== 0) return `${e} --png: status ${r.status}: ${r.stdout}${r.stderr}`;
        const d = fs.readFileSync(png);
        if (d.readUInt32BE(0) !== 0x89504e47 || d.length < 2000) return `${e}: not much of a PNG`;
      }
      return null;
    },
    'axiom test': () => {
      const r = spawnSync(compiler, ['test', path.join(root, 'std/str.ax'), '--target=aarch64-macos'], { encoding: 'utf8', cwd: root });
      if (!onMac) return r.status !== 0 && /E9001/.test(r.stderr) ? null : `axiom test: status ${r.status}: ${r.stderr}`;
      return r.status === 0 && /passed, 0 failed/.test(r.stdout) ? null : `axiom test: status ${r.status}: ${r.stdout}${r.stderr}`;
    },
  }, {
    supported: () => (/unknown target/.test(build(path.join(root, 'examples/wordcount/wordcount.ax'), 'probe').stderr) ? 'the bootstrap compiler has no macOS target' : null),
    note: onMac ? '' : 'built, not run: no Mac',
    timed: false,
  });
}

// Windows (spec §14.1; the compiler only, not the bootstrap one). Programs and the
// compiler itself are cross-built for --target=x86_64-windows and their PE
// headers checked; with Wine installed they also run (the whole suite runs
// on Windows with AXIOM_NATIVE_TARGET=x86_64-windows AXIOM_NATIVE_EXEC=wine64).
if (!skipPlatform) {
  const out = (n) => path.join(tmp, `windows-${n}.exe`);
  const env = { ...process.env, AXIOM_CACHE: path.join(tmp, 'windows-cache'), WINEDEBUG: process.env.WINEDEBUG || '-all' };
  const build = (src, n, extra = []) => spawnSync(compiler, ['build', src, '--target=x86_64-windows', '-o', out(n), ...extra],
    { encoding: 'utf8', cwd: root, env });
  const wine = ['wine64', 'wine', '/usr/lib/wine/wine64'].find((w) => spawnSync(w, ['--version'], { encoding: 'utf8' }).status === 0);
  // The headers: MZ, PE, x86-64, a console program with ASLR, and its sections.
  const checkPE = (f) => {
    const d = fs.readFileSync(f);
    if (d.readUInt16LE(0) !== 0x5a4d) return 'no MZ header';
    const pe = d.readUInt32LE(0x3c);
    if (d.readUInt32LE(pe) !== 0x4550 || d.readUInt16LE(pe + 4) !== 0x8664) return 'not a PE file for x86-64';
    const opt = pe + 24;
    if (d.readUInt16LE(opt) !== 0x20b || d.readUInt16LE(opt + 68) !== 3) return 'not a PE32+ console program';
    if ((d.readUInt16LE(opt + 70) & 0x60) !== 0x60) return 'no (high-entropy) ASLR';
    const names = [];
    for (let k = 0; k < d.readUInt16LE(pe + 6); k++) names.push(d.toString('latin1', opt + 240 + 40 * k, opt + 248 + 40 * k).replace(/\0+$/, ''));
    return names.join(' ') === '.text .rdata .idata .data .reloc' ? null : `sections ${names.join(' ')}`;
  };
  const run = (n, args = []) => spawnSync(wine, [out(n), ...args], { encoding: 'utf8', cwd: root, env });
  const xvfb = ['xvfb-run'].find((x) => spawnSync('sh', ['-c', `command -v ${x}`]).status === 0);
  const vulkan = ['/usr/share/vulkan/icd.d', '/etc/vulkan/icd.d'].some((d) => fs.existsSync(d) && fs.readdirSync(d).length > 0);
  // An 8-bit RGBA PNG (what std/graphics/image/png writes for RGBA images).
  const decodePNG = (d) => {
    let at = 8, width = 0, height = 0;
    const idat = [];
    while (at < d.length) {
      const len = d.readUInt32BE(at), type = d.toString('latin1', at + 4, at + 8);
      if (type === 'IHDR') { width = d.readUInt32BE(at + 8); height = d.readUInt32BE(at + 12); }
      if (type === 'IDAT') idat.push(d.subarray(at + 8, at + 8 + len));
      at += 12 + len;
    }
    const raw = zlib.inflateSync(Buffer.concat(idat));
    const data = Buffer.alloc(width * height * 4), stride = width * 4;
    for (let y = 0; y < height; y++) {
      const f = raw[y * (stride + 1)], row = raw.subarray(y * (stride + 1) + 1, (y + 1) * (stride + 1));
      for (let x = 0; x < stride; x++) {
        const a = x >= 4 ? data[y * stride + x - 4] : 0, b = y > 0 ? data[(y - 1) * stride + x] : 0;
        const c = x >= 4 && y > 0 ? data[(y - 1) * stride + x - 4] : 0;
        const p = a + b - c, pa = Math.abs(p - a), pb = Math.abs(p - b), pc = Math.abs(p - c);
        const pred = [0, a, b, (a + b) >> 1, pa <= pb && pa <= pc ? a : pb <= pc ? b : c][f];
        data[y * stride + x] = (row[x] + pred) & 255;
      }
    }
    return { width, height, data };
  };
  await section('windows', {
    'thread-locals and output': () => {
      const src = path.join(tmp, 'windows-hello.ax');
      fs.writeFileSync(src, 'import "std/io" as io;\n\nthreadlocal let mut tl: u64 = 41;\n\nexport fn main() i32 {\n' +
        '    tl += 1;\n    io.stdout().println("hello") catch return 1;\n    return @int_cast(i32, tl) - 42;\n}\n');
      const c = build(src, 'hello', ['-O2']);
      if (c.status !== 0) return `build failed:\n${c.stdout}${c.stderr}`;
      const p = checkPE(out('hello'));
      if (p || !wine) return p;
      const r = run('hello');
      return r.status === 0 && r.stdout === 'hello\n' ? null : `status ${r.status}: ${r.stdout}${r.stderr}`;
    },
    'debug symbols (a PDB)': () => {
      const c = build(path.join(root, 'examples/pi/pi.ax'), 'pdb', ['-O2']);
      if (c.status !== 0) return `build failed:\n${c.stdout}${c.stderr}`;
      const exe = fs.readFileSync(out('pdb'));
      const pdbFile = out('pdb').replace(/\.exe$/, '.pdb');
      if (!fs.existsSync(pdbFile)) return 'no PDB';
      // the executable's debug directory: a CodeView (RSDS) entry naming the PDB
      const pe = exe.readUInt32LE(0x3c), opt = pe + 24, nsec = exe.readUInt16LE(pe + 6);
      const dirRva = exe.readUInt32LE(opt + 112 + 6 * 8);
      const fileOf = (rva) => {
        for (let k = 0; k < nsec; k++) {
          const s = opt + 240 + 40 * k, va = exe.readUInt32LE(s + 12), raw = exe.readUInt32LE(s + 20);
          if (rva >= va && rva < va + exe.readUInt32LE(s + 8)) return raw + rva - va;
        }
        return -1;
      };
      const dir = fileOf(dirRva);
      if (dir < 0 || exe.readUInt32LE(dir + 12) !== 2) return 'no CodeView debug directory';
      const rsds = exe.readUInt32LE(dir + 24);
      if (exe.toString('latin1', rsds, rsds + 4) !== 'RSDS') return 'no RSDS record';
      const guid = exe.subarray(rsds + 4, rsds + 20);
      const name = exe.toString('utf8', rsds + 24, exe.indexOf(0, rsds + 24));
      if (!/pdb\.pdb$/.test(name)) return `the record names ${name}`;
      // the PDB: an MSF whose info stream (1) has the same GUID and age
      const p = fs.readFileSync(pdbFile);
      if (p.toString('latin1', 0, 24) !== 'Microsoft C/C++ MSF 7.00') return 'not an MSF file';
      const bs = p.readUInt32LE(32), dirBlock = p.readUInt32LE(p.readUInt32LE(52) * bs);
      const d = dirBlock * bs, nstreams = p.readUInt32LE(d);
      let at = d + 4 + nstreams * 4;
      for (let s = 0; s < 1; s++) at += 4 * Math.ceil(p.readUInt32LE(d + 4 + s * 4) / bs);
      const info = p.readUInt32LE(at) * bs;
      if (p.readUInt32LE(info) !== 20000404) return 'bad info stream';
      if (!p.subarray(info + 12, info + 28).equals(guid)) return 'the GUIDs differ';
      if (p.readUInt32LE(info + 8) !== exe.readUInt32LE(rsds + 20)) return 'the ages differ';
      // the functions' names are in it (the publics), and their files (/names)
      const text = p.toString('latin1');
      return text.includes('examples/pi/pi.pi_digits') && /examples[\\/]pi[\\/]pi\.ax/.test(text) ? null : 'names or files missing';
    },
    'pi to 2000 places': () => {
      const c = build(path.join(root, 'examples/pi/pi.ax'), 'pi', ['-O2']);
      if (c.status !== 0) return `build failed:\n${c.stdout}${c.stderr}`;
      const p = checkPE(out('pi'));
      if (p || !wine) return p;
      const r = run('pi', ['-p', '-n', '100', path.join(tmp, 'windows-pi.pdf')]);
      const want = '3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679';
      if (r.status !== 0 || !r.stdout.startsWith(want)) return `status ${r.status}: ${r.stdout}${r.stderr}`;
      return fs.readFileSync(path.join(tmp, 'windows-pi.pdf')).subarray(0, 5).toString() === '%PDF-' ? null : 'no PDF';
    },
    'Direct3D 11 and Direct2D': () => {
      const c = build(path.join(root, 'examples/directx/directx.ax'), 'directx', ['-O2']);
      if (c.status !== 0) return `build failed:\n${c.stdout}${c.stderr}`;
      const p = checkPE(out('directx'));
      if (p || !wine || !xvfb) return p;
      // one frame offscreen (Wine draws with OpenGL, so it needs a display)
      const png = path.join(tmp, 'windows-triangle.png');
      const r = spawnSync(xvfb, ['-a', wine, out('directx'), '--png', png], { encoding: 'utf8', cwd: root, env });
      if (r.status !== 0 || !fs.existsSync(png)) return `status ${r.status}: ${r.stdout}${r.stderr}`;
      const img = decodePNG(fs.readFileSync(png));
      const px = (x, y) => Array.from(img.data.subarray((y * img.width + x) * 4, (y * img.width + x) * 4 + 3));
      const near = (a, b) => a.every((v, i) => Math.abs(v - b[i]) <= 3);
      const top = (a, k) => a[k] > 150 && a.every((v, i) => i === k || v < 100);
      if (!near(px(5, 5), [20, 20, 31])) return `background ${px(5, 5)}`;
      if (!top(px(128, 40), 0) || !top(px(52, 172), 2) || !top(px(204, 172), 1)) {
        return `corners ${px(128, 40)} ${px(52, 172)} ${px(204, 172)}`;
      }
      // the Direct2D caption: a dark box along the bottom with white text
      if (!px(12, 240).every((v) => v < 20)) return `caption box ${px(12, 240)}`;
      let white = 0;
      for (let y = 226; y < 246; y++) for (let x = 20; x < 236; x++) if (px(x, y).every((v) => v > 200)) white++;
      return white > 40 ? null : `caption text: ${white} white pixels`;
    },
    'Direct3D 12, XInput and XAudio2': () => {
      const c = build(path.join(root, 'examples/game/game.ax'), 'game', ['-O2']);
      if (c.status !== 0) return `build failed:\n${c.stdout}${c.stderr}`;
      const p = checkPE(out('game'));
      // under Wine, Direct3D 12 is vkd3d over Vulkan (Mesa's lavapipe will do)
      if (p || !wine || !xvfb || !vulkan) return p;
      const png = path.join(tmp, 'windows-game.png');
      const r = spawnSync(xvfb, ['-a', wine, out('game'), '--png', png], { encoding: 'utf8', cwd: root, env });
      if (r.status !== 0 || !fs.existsSync(png)) return `status ${r.status}: ${r.stdout}${r.stderr}`;
      const img = decodePNG(fs.readFileSync(png));
      const px = (x, y) => Array.from(img.data.subarray((y * img.width + x) * 4, (y * img.width + x) * 4 + 4));
      const [bg, ship, orb] = [px(0, 0), px(128, 128), px(128, 51)];
      if (!(bg[0] < 30 && bg[1] < 30 && bg[2] < 40 && bg[3] === 255)) return `background ${bg}`;
      if (!(ship[0] < 100 && ship[1] > 140 && ship[2] > 150)) return `ship ${ship}`;
      if (!(orb[0] > 200 && orb[1] > 150 && orb[2] < 120)) return `orb ${orb}`;
      // the GPU, controllers (none here) and sound (a chime that plays, when
      // there is an audio device)
      const k = spawnSync(xvfb, ['-a', wine, out('game'), '--check'], { encoding: 'utf8', cwd: root, env });
      if (k.status !== 0 || !/^direct3d 12: ok\ncontrollers: 0\naudio: (ok|none)\n/.test(k.stdout)) {
        return `check: status ${k.status}: ${k.stdout}${k.stderr}`;
      }
      // playing by itself in a window
      const w = spawnSync(xvfb, ['-a', wine, out('game'), '--frames', '120', '--autopilot'],
        { encoding: 'utf8', cwd: root, env, timeout: 120000 });
      return w.status === 0 && /^120 frames, \d+ orbs\n/.test(w.stdout) ? null : `window: status ${w.status}: ${w.stdout}${w.stderr}`;
    },
    'Direct3D: textures, depth, compute, Direct2D on 12, DXIL (cmd/axiom/internal/tests/windows)': () => {
      const files = ['cmd/axiom/internal/tests/windows/d3d11.ax', 'cmd/axiom/internal/tests/windows/d3d12.ax'];
      for (const f of files) {
        const c = spawnSync(compiler, ['check', f, '--target=x86_64-windows'], { encoding: 'utf8', cwd: root, env });
        if (c.status !== 0) return `${f}: check failed:\n${c.stdout}${c.stderr}`;
      }
      // On Windows they run on the GPU. Under Wine only Direct3D 11's do:
      // Direct3D 12's draw with Direct2D through D3D11On12, which Wine lacks.
      const onWindows = process.platform === 'win32';
      const runs = onWindows ? files : wine && xvfb ? files.slice(0, 1) : [];
      for (const f of runs) {
        const args = ['test', f, '--target=x86_64-windows'];
        const r = onWindows ? spawnSync(compiler, args, { encoding: 'utf8', cwd: root, env })
          : spawnSync(xvfb, ['-a', compiler, ...args], { encoding: 'utf8', cwd: root, env });
        if (r.status !== 0) return `${f}: status ${r.status}: ${r.stdout}${r.stderr}`;
      }
      return null;
    },
    'the compiler': () => {
      const c = build(path.join(root, 'cmd/axiom/src/main.ax'), 'axiom', ['-O1']);
      if (c.status !== 0) return `build failed:\n${c.stdout}${c.stderr}`;
      const p = checkPE(out('axiom'));
      if (p || !wine) return p;
      // it builds a program that runs
      const b = run('axiom', ['build', 'examples/wordcount/wordcount.ax', '--std=std', '--target=x86_64-windows', '-o', out('wc')]);
      if (b.status !== 0) return `the Windows compiler failed: ${b.stdout}${b.stderr}`;
      const r = run('wc', ['README.md']);
      return r.status === 0 ? null : `wordcount: status ${r.status}: ${r.stdout}${r.stderr}`;
    },
  }, {
    supported: () => (/unknown target/.test(build(path.join(root, 'examples/wordcount/wordcount.ax'), 'probe').stderr) ? 'the bootstrap compiler has no Windows target' : null),
    note: wine ? '' : 'built, not run: no Wine',
    timed: false,
  });
}

// Packages (spec §4.6): builds from a package directory, dependency
// visibility, name conflicts, manifests, git dependencies, `axiom new`.
{
  const pk = path.join(root, 'cmd/axiom/internal/tests/packages');
  const env = { ...process.env, AXIOM_CACHE: path.join(tmp, 'cache') };
  const ph = (args, cwd) => spawnSync(compiler, [...args, ...extraFlags], { encoding: 'utf8', cwd: cwd || root, env });
  const git = (args, cwd) => spawnSync('git', args, { encoding: 'utf8', cwd,
    env: { ...env, GIT_AUTHOR_NAME: 't', GIT_AUTHOR_EMAIL: 't@t', GIT_COMMITTER_NAME: 't', GIT_COMMITTER_EMAIL: 't@t' } });
  const tgt = native ? [nativeTarget] : [];
  async function buildRun(dir, cwd) {
    const out = path.join(tmp, 'pkg-' + path.basename(dir) + (native ? '' : '.wasm'));
    const c = ph(['build', dir, '--format=agent-json', '-o', out, ...tgt], cwd);
    if (c.status !== 0) return { problem: `compilation failed:\n${c.stdout}${c.stderr}` };
    const r = native ? await runNative(out) : await runWasm(out);
    if (r.stderr) process.stderr.write(r.stderr);
    return { out: r.out.join('\n'), trapped: r.trapped };
  }
  function tested(dir, cwd) {
    const c = ph(['test', dir, '--format=agent-json', ...tgt], cwd);
    try { return JSON.parse(c.stdout); } catch { return { status: 'bad', output: c.stdout + c.stderr, tests: [] }; }
  }
  function errorCodes(dir) {
    const c = ph(['check', dir, '--format=agent-json']);
    try { return JSON.parse(c.stdout).diagnostics.map((x) => x.code); } catch { return [`bad output: ${c.stdout}${c.stderr}`]; }
  }
  const names = (j) => j.tests.map((t) => t.name).sort().join(',');
  await section('packages', {
    'build and run app': async () => {
      const r = await buildRun(path.join(pk, 'app'));
      return r.problem || (r.out !== '81 100 5 20' ? `output: ${r.out}` : null);
    },
    'test runs only the root package': async () => {
      const j = tested('.', path.join(pk, 'app'));
      return j.status === 'pass' && names(j) === 'app uses mathlib' ? null : JSON.stringify(j);
    },
    'test loads every source of the package': async () => {
      const j = tested(path.join(pk, 'mathlib'));
      return j.status === 'pass' && names(j) === 'square,sum' ? null : JSON.stringify(j);
    },
    'undeclared dependency': async () => (errorCodes(path.join(pk, 'errors/undeclared')).includes('E0214') ? null : errorCodes(path.join(pk, 'errors/undeclared')).join()),
    'package name conflict': async () => (errorCodes(path.join(pk, 'errors/conflict')).includes('E0213') ? null : 'no E0213'),
    'missing path dependency': async () => (errorCodes(path.join(pk, 'errors/missingdep')).includes('E0217') ? null : 'no E0217'),
    'invalid manifest': async () => {
      const c = errorCodes(path.join(pk, 'errors/badmanifest'));
      return c.length === 4 && c.every((x) => x === 'E0215') ? null : c.join();
    },
    'a package that needs a newer compiler': async () => {
      const c = errorCodes(path.join(pk, 'errors/newer'));
      return c.length === 1 && c[0] === 'E0220' ? null : c.join();
    },
    'git dependency': async () => {
      const repo = path.join(tmp, 'gitutil');
      fs.cpSync(path.join(pk, 'util'), repo, { recursive: true });
      for (const a of [['init', '-q'], ['add', '.'], ['commit', '-q', '-m', 'util']]) {
        const r = git(a, repo);
        if (r.status !== 0) return `git ${a[0]}: ${r.stderr}`;
      }
      const rev = git(['rev-parse', 'HEAD'], repo).stdout.trim();
      const app = path.join(tmp, 'gitapp');
      fs.mkdirSync(path.join(app, 'src'), { recursive: true });
      const manifest = (r) => `[package]\nname = "gitapp"\n\n[dependencies]\nutil = { git = "${repo}", rev = "${r}" }\n`;
      fs.writeFileSync(path.join(app, 'axiom.toml'), manifest(rev));
      fs.writeFileSync(path.join(app, 'src/main.ax'),
        'import "pkg:util" as util;\nimport "std/io" as io;\n\nexport fn main() i32 {\n' +
        '    io.stdout().print_i64(util.clamp(42, 0, 7)) catch return 1;\n    return 0;\n}\n');
      for (let i = 0; i < 2; i++) {  // the second build uses the cache
        const r = await buildRun(app);
        if (r.problem || r.out !== '7') return r.problem || `output: ${r.out}`;
      }
      fs.writeFileSync(path.join(app, 'axiom.toml'), manifest('0'.repeat(40)));
      const codes = errorCodes(app);
      if (!codes.includes('E0216')) return `bad rev: ${codes.join()}`;
      // a revision that is not a full hash names a path in the cache (':/text' picks a commit by its message)
      fs.writeFileSync(path.join(app, 'axiom.toml'), manifest(':/../../escape'));
      const short = errorCodes(app);
      return short.length === 1 && short[0] === 'E0215' ? null : `rev ':/...': ${short.join()}`;
    },
    'no manifest from a directory anyone may write to': async () => {
      const shared = path.join(tmp, 'shared');
      fs.mkdirSync(path.join(shared, 'work'), { recursive: true });
      fs.chmodSync(shared, 0o1777);
      fs.writeFileSync(path.join(shared, 'axiom.toml'), '[package]\nname = "planted"\n\n[dependencies]\nx = { path = "missing" }\n');
      fs.writeFileSync(path.join(shared, 'work/main.ax'), 'export fn main() i32 {\n    return 0;\n}\n');
      const codes = errorCodes(path.join(shared, 'work/main.ax'));
      return codes.length === 0 ? null : codes.join();
    },
    'axiom new': async () => {
      const c = ph(['new', 'fresh'], tmp);
      if (c.status !== 0) return `status ${c.status}: ${c.stdout}${c.stderr}${c.error || ''}`;
      const j = tested('.', path.join(tmp, 'fresh'));
      return j.status === 'pass' && names(j) === 'add' ? null : JSON.stringify(j);
    },
  });
}

// The node host itself (std/internal/runtime/host.mjs), driven without a program.
if (!native) {
  await section('host', {
    // A socket request completes with a store and then a notify (see
    // std/internal/runtime/net_worker.mjs). A thread that sees the store before it sleeps
    // returns at once, and that request's notify comes later: it must not
    // complete the thread's next request, which would return the previous
    // result (a read returning a write's count, with stale data).
    'net requests wait for their own completion': async () => {
      const helper = new Worker(`
        const { parentPort } = require('worker_threads');
        parentPort.on('message', ({ ctrl, delay, result }) => setTimeout(() => {
          const c = new Int32Array(ctrl);
          if (result !== undefined) { Atomics.store(c, 1, result); Atomics.store(c, 0, 1); }
          Atomics.notify(c, 0);
        }, delay));`, { eval: true });
      let requests = 0;
      const port = {
        postMessage(m) {
          const c = new Int32Array(m.ctrl);
          if (++requests === 1) {
            // done before the caller waits; notified 50 ms later
            Atomics.store(c, 1, m.payload.length);
            Atomics.store(c, 0, 1);
            helper.postMessage({ ctrl: m.ctrl, delay: 50 });
          } else {
            new Uint8Array(m.data).set([7, 7, 7]);
            helper.postMessage({ ctrl: m.ctrl, delay: 300, result: 3 });
          }
        },
        close() {},
      };
      const host = createHost({ netPort: port });
      host.setMemory(new WebAssembly.Memory({ initial: 1 }));
      const net = host.imports.axiom;
      const wrote = net.ax_sock_write(3, 0, 5);
      const got = net.ax_sock_read(3, 64, 16);
      const data = Array.from(new Uint8Array(host.memory().buffer, 64, 3));
      host.close();
      await helper.terminate();
      return wrote === 5 && got === 3 && data.join() === '7,7,7' ? null : `write returned ${wrote}, read ${got} [${data}]`;
    },
  });
}

if (listOnly) process.exit(0);
for (const f of failures) console.log('\n' + f);
if (skip && !verbose) console.log(`\n${skip} skipped (--verbose names them)`);
console.log(`\n${pass} passed, ${fail} failed${skip ? `, ${skip} skipped` : ''}`);
process.exit(fail ? 1 : 0);
