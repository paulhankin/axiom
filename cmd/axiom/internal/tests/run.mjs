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
//   // linux-only              natively, skip on macOS and Windows targets
//   // selfhost-only: <why>    skip with the bootstrap compiler (whose LLVM differs:
//                              it folds an integer round trip of a pointer, say)
//   // x86-64-flags: --cpu=x86-64  extra flags for a native build on an x86-64 host
//   // wasm-flags: --features=+tail-call  extra flags for a WebAssembly build
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
import crypto from 'crypto';
import fs from 'fs';
import path from 'path';
import zlib from 'zlib';
import os from 'os';
import { Worker } from 'worker_threads';
import { createHost } from '../../../../std/internal/runtime/host.mjs';
import { checkMachO } from './macho.mjs';
import { checkPDB } from './pdb.mjs';
import { checkPE, checkPEFile } from './pe.mjs';
import { loadImage, checkDebug, addrsOfLine, subprogramAt, checkVars, checkAxVars, readDwarf, frameAt } from './dwarf.mjs';

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
// Build-only (AXIOM_NATIVE_EXEC=none, for a target nothing here can run:
// AXIOM_NATIVE_TARGET=x86_64-macos, aarch64-windows, ...): the sections that
// build programs (option buildOnly) build them and check each executable's
// structure (checkBuilt: macho.mjs, pe.mjs) instead of running it; the
// others are skipped.
const buildOnly = native && process.env.AXIOM_NATIVE_EXEC === 'none';
const buildNote = buildOnly ? 'built and checked, not run' : '';
const execWrap = (buildOnly ? '' : process.env.AXIOM_NATIVE_EXEC || '').split(' ').filter((x) => x);
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
  const d = { expect: [], trap: false, errors: [], warnings: [], flags: [], x64Flags: [], wasmFlags: [], wasmOnly: false, nativeOnly: false, linuxOnly: false, selfhostOnly: null, links: [], header: null };
  for (const [i, line] of src.split('\n').entries()) {
    let m;
    if ((m = line.match(/\/\/ expect: ?(.*)$/))) d.expect.push(m[1]);
    else if ((m = line.match(/\/\/ expect-warning: (W\d+)/))) d.warnings.push(`${m[1]} at line ${i + 1}`);
    else if (/\/\/ expect-trap\b/.test(line)) d.trap = true;
    else if ((m = line.match(/\/\/ expect-error: (E\d+)/))) d.errors.push(m[1]);
    else if ((m = line.match(/\/\/ flags: (.*)$/))) d.flags.push(...m[1].trim().split(/\s+/));
    else if ((m = line.match(/\/\/ x86-64-flags: (.*)$/))) d.x64Flags.push(...m[1].trim().split(/\s+/));
    else if ((m = line.match(/\/\/ wasm-flags: (.*)$/))) d.wasmFlags.push(...m[1].trim().split(/\s+/));
    else if (/\/\/ wasm-only\b/.test(line)) d.wasmOnly = true;
    else if (/\/\/ native-only\b/.test(line)) d.nativeOnly = true;
    else if (/\/\/ linux-only\b/.test(line)) d.linuxOnly = true;
    else if ((m = line.match(/\/\/ selfhost-only\b:? ?(.*)$/))) d.selfhostOnly = m[1] || 'not for the bootstrap compiler';
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
  // (a program that ends by calling exit, as a hosted C program does when
  // main returns, ends this run alone: the host's exit throws WasmExit)
  const host = createHost({
    exit: (code) => { throw new WasmExit(code); },
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
    print_str: (p, n) => line(dec.decode(new Uint8Array(mem.buffer, Number(p), Number(n)))),  // (BigInts on wasm64)
    print_bool: b => line(b ? 'true' : 'false'),
  };
  const instance = host.instantiate(fs.readFileSync(file), { env });
  mem = instance.exports.memory || host.memory();
  const lines = () => { const o = text.split('\n'); if (o[o.length - 1] === '') o.pop(); return o; };
  try {
    (instance.exports.run || instance.exports.main)();
    return { out: lines(), trapped: false, stderr: errText };
  } catch (e) {
    if (e instanceof WasmExit) return { out: lines(), trapped: false, stderr: errText };
    if (e instanceof WebAssembly.RuntimeError || e instanceof RangeError) return { out: lines(), trapped: true, msg: e.message, stderr: errText };
    throw e;
  } finally {
    host.close();
  }
}

class WasmExit extends Error {
  constructor(code) {
    super(`exit ${code}`);
    this.code = code;
  }
}

// A PE file's sections and data directories, for the checks below.
function peLayout(b) {
  const pe = b.readUInt32LE(0x3c), nsec = b.readUInt16LE(pe + 6), optSize = b.readUInt16LE(pe + 20);
  const opt = pe + 24, secs = [];
  for (let i = 0; i < nsec; i++) {
    const o = opt + optSize + i * 40;
    secs.push({ va: b.readUInt32LE(o + 12), vsize: b.readUInt32LE(o + 8), raw: b.readUInt32LE(o + 20) });
  }
  const off = (rva) => { const s = secs.find((x) => rva >= x.va && rva < x.va + x.vsize); return s ? s.raw + (rva - s.va) : -1; };
  const dir = (k) => ({ rva: b.readUInt32LE(opt + 112 + k * 8), size: b.readUInt32LE(opt + 112 + k * 8 + 4) });
  return { off, dir };
}

// The names a PE file imports, by DLL.
function peImports(b) {
  const { off, dir } = peLayout(b);
  const cstr = (o) => b.subarray(o, b.indexOf(0, o)).toString();
  const res = {};
  for (let d = off(dir(1).rva); b.readUInt32LE(d + 12) !== 0; d += 20) {
    const names = res[cstr(off(b.readUInt32LE(d + 12)))] = [];
    for (let t = off(b.readUInt32LE(d)); ; t += 8) {
      const e = Number(b.readBigUInt64LE(t));
      if (!e) break;
      names.push(cstr(off(e) + 2));
    }
  }
  return res;
}

// The unwind information of the functions with a frame register in a PE
// file (those whose stack pointer moves: a variable length array): every
// register they save must be restored relative to the frame register, not
// to rsp (Windows x64 unwinding: x64emit.ax, dyn_unwind_info). A problem, or
// null; it is one if there is no such function.
function checkDynUnwind(b) {
  const { off, dir } = peLayout(b);
  const ex = dir(3);
  let found = 0;
  for (let e = off(ex.rva); e < off(ex.rva) + ex.size; e += 12) {
    const u = off(b.readUInt32LE(e + 8));
    const n = b[u + 2], fr = b[u + 3] & 15, fo = b[u + 3] >> 4;
    // (a chained entry: the code after a sibling call's jump, whose first entry is checked)
    if (!fr || (b[u] >> 3) & 4) continue;
    found++;
    if (fr !== 5) return `frame register ${fr}, not rbp`;
    const codes = [];
    for (let j = 0; j < n;) {
      const at = b[u + 4 + 2 * j], op = b[u + 5 + 2 * j] & 15, info = b[u + 5 + 2 * j] >> 4;
      const size = op === 5 || op === 9 ? 3 : (op === 4 || op === 8 || op === 1 ? 2 : 1);
      const val = size === 3 ? b.readUInt32LE(u + 6 + 2 * j) : (size === 2 ? b.readUInt16LE(u + 6 + 2 * j) : 0);
      codes.push({ at, op, info, val });
      j += size;
    }
    for (let k = 1; k < codes.length; k++) if (codes[k].at > codes[k - 1].at) return 'unwind codes out of order';
    const setfp = codes.findIndex((c) => c.op === 3);
    if (setfp < 0) return 'no UWOP_SET_FPREG';
    // before it in the list (applied first): only saves relative to the frame base
    for (const c of codes.slice(0, setfp)) {
      if (c.op !== 5 && c.op !== 9) return `unwind code ${c.op} before UWOP_SET_FPREG (relative to rsp)`;
      if (c.val + (c.op === 9 ? 16 : 8) > 16 * fo) return 'a saved register above the frame register';
    }
    const rest = codes.slice(setfp + 1);
    const last = rest[rest.length - 1];
    if (!last || last.op !== 0 || last.info !== 5) return 'the last unwind code is not the push of rbp';
    const lift = rest.slice(0, -1).reduce((s, c) => s + (c.op === 2 ? (c.info + 1) * 8 : c.op === 1 && c.info === 0 ? c.val * 8 : 1e9), 0);
    if (lift !== 16 * fo) return `rsp is lifted by ${lift} to rbp, not ${16 * fo}`;
  }
  return found ? null : 'no function with a frame register';
}

// A build-only run's check of an executable for the native target: its
// structure (Mach-O or PE, with its PDB) as the system's loader would see
// it; null when it is sound.
function checkBuilt(exe) {
  const t = process.env.AXIOM_NATIVE_TARGET || '';
  let errs = [];
  if (/macos|darwin/.test(t)) errs = checkMachO(fs.readFileSync(exe), { x86_64: /x86_64/.test(t) }).errs;
  else if (/windows/.test(t)) errs = checkPEFile(exe, { arm64: /aarch64/.test(t) }).errs;
  else if (fs.readFileSync(exe).readUInt32BE(0) !== 0x7f454c46) errs = ['not an ELF file'];
  return errs.length ? `the executable: ${errs.join('\n')}` : null;
}

// Before a Windows executable runs (under Wine, say): checkBuilt's checks,
// the x64 and ARM64 unwind information of every function included
// (pe.mjs), and no import of the system's C library (msvcrt.dll, the
// universal C runtime): programs use the compiler's own (docs/c.md).
// null when it is sound.
function checkWindowsExe(exe) {
  if (!/windows/.test(process.env.AXIOM_NATIVE_TARGET || '')) return null;
  const r = checkPEFile(exe, { arm64: /aarch64/.test(process.env.AXIOM_NATIVE_TARGET) });
  const crt = (r.imports || []).filter((x) => /^(msvcrt|ucrtbase|api-ms-win-crt-[a-z-]+-l1-1-0)\.dll:/i.test(x));
  const errs = [...r.errs, ...(crt.length ? [`imports from the system's C library: ${crt.join(' ')}`] : [])];
  return errs.length ? `the executable: ${errs.join('\n')}` : null;
}

async function runNative(exe) {
  const bad = checkWindowsExe(exe);
  if (bad) return { out: [], trapped: true, msg: bad };
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
  if (buildOnly && !opts.buildOnly) {
    for (const [n] of entries) record(n, { skip: true, reason: 'a build-only run (AXIOM_NATIVE_EXEC=none)' });
    return;
  }
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
  if (native && d.linuxOnly && noC) return { skip: true, reason: 'Linux targets only' };
  if (d.selfhostOnly && bootstrapCompiler()) return { skip: true, reason: d.selfhostOnly };
  if (native && noC && (d.links.length || /extern "C"/.test(src))) return { skip: true, reason: 'links C code' };
  const dir = path.join(tmp, path.basename(path.dirname(f)) + '-' + path.basename(f, '.ax'));
  fs.mkdirSync(dir);
  const out = path.join(dir, path.basename(f, '.ax') + (native ? '' : '.wasm'));
  const args = ['build', f, '--format=agent-json', '-o', out, ...d.flags, ...extraFlags];
  if (native) args.push(nativeTarget, `--link=${path.join(root, noC ? 'cmd/axiom/internal/tests/native_rt.ax' : 'cmd/axiom/internal/tests/native_rt.c')}`);
  if (native && process.arch === 'x64' && !process.env.AXIOM_NATIVE_TARGET) args.push(...d.x64Flags);
  if (!native) args.push(...d.wasmFlags);
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
  if (buildOnly) return checkBuilt(out);
  const r = native ? await runNative(out) : await runWasm(out);
  let problem = null;
  if (d.trap && !r.trapped) problem = 'expected a trap, but run completed';
  if (!d.trap && r.trapped) problem = `unexpected trap: ${r.msg}`;
  const want = d.expect.join('\n'), got = r.out.join('\n');
  if (!problem && want !== got) problem = `output mismatch\n--- expected\n${want}\n--- got\n${got}`;
  return { problem, stderr: r.stderr };
}), { parallel: true, buildOnly: true, note: buildNote });

// The C preprocessor (docs/c-frontend.md): each cmd/axiom/internal/tests/c/pp/*.c
// is preprocessed (--emit=c-pp) with its /* flags: */; the output must be
// the .pp file next to it, or the build must fail with each
// /* expect-error: C1xxx */ code. /* expect-warning: C4xxx */ codes must be
// reported. __DATE__ and __TIME__ are fixed (SOURCE_DATE_EPOCH).
const cDir = path.join(root, 'cmd/axiom/internal/tests/c/pp');
const cFiles = fs.existsSync(cDir) ? fs.readdirSync(cDir).filter((f) => f.endsWith('.c')).sort().map((f) => path.join(cDir, f)) : [];
await section('', byFile(cFiles, '[c-pp]', async (f) => {
  if (bootstrapCompiler()) return { skip: true, reason: 'the C front end is the compiler\'s' };
  const src = fs.readFileSync(f, 'utf8');
  const flags = [...src.matchAll(/\/\* flags: (.*?) \*\//g)].flatMap((m) => m[1].trim().split(/\s+/));
  const errors = [...src.matchAll(/\/\* expect-error: (C\d+) \*\//g)].map((m) => m[1]);
  const warnings = [...src.matchAll(/\/\* expect-warning: (C\d+) \*\//g)].map((m) => m[1]);
  const env = { ...process.env, SOURCE_DATE_EPOCH: '1700000000' };
  const c = await spawnAsync(compiler, ['build', path.basename(f), '--emit=c-pp', ...flags],
    { encoding: 'utf8', timeout: compileTimeout, cwd: cDir, env });
  if (c.error && c.error.code === 'ETIMEDOUT') return `the compiler did not finish in ${compileTimeout / 1000} s`;
  const codes = (sev) => [...c.stderr.matchAll(new RegExp(`: ${sev}\\[(C\\d+)\\]`, 'g'))].map((m) => m[1]);
  for (const w of warnings) if (!codes('warning').includes(w)) return `expected warning ${w}, got [${codes('warning').join(', ')}]`;
  if (errors.length) {
    if (c.status === 0) return `expected errors ${errors.join(', ')}, but preprocessing succeeded`;
    if (c.status !== 1) return `the compiler failed (status ${c.status}, signal ${c.signal}):\n${c.stderr}`;
    for (const e of errors) if (!codes('error').includes(e)) return `expected ${e}, got [${codes('error').join(', ')}]:\n${c.stderr}`;
    return null;
  }
  if (c.status !== 0) return `preprocessing failed:\n${c.stdout}${c.stderr}`;
  const want = fs.readFileSync(f.replace(/\.c$/, '.pp'), 'utf8');
  return want === c.stdout ? null : `output mismatch\n--- expected\n${want}--- got\n${c.stdout}`;
}), { parallel: true });

// C programs (docs/c-frontend.md): each cmd/axiom/internal/tests/c/run/*.c is
// built (for wasm32, or with --native for this machine, at the suite's
// optimization level) with its /* flags: */ and run; its output must be its
// /* expect: */ lines, or the build must fail with each /* expect-error: C3xxx */
// code; it must warn with each /* expect-warning: C4xxx */ code. The tests
// print through host.h (next to run/); natively its functions are
// c/host_native.c's, compiled by the system's C compiler, or for Windows and
// macOS c/host_c.c's, a second translation unit over the platform's C library
// (tests marked /* linux-only */ need the former; /* not-windows */ ones are
// skipped for a Windows target; /* wasm-flags: */ are more flags for a
// WebAssembly build). /* units: a.c */ names more
// translation units, next to the test (c/run/abi/). AXIOM_C_TARGET=wasm64
// builds them for wasm64 instead of wasm32. A test marked /* check-only */ is
// only checked (axiom check, with its own --target), not built or run.
// The larger programs of c/prog/ are built and run the same way ([c-prog]).
const cRunDir = path.join(root, 'cmd/axiom/internal/tests/c/run');
const cFilesIn = (dir) => fs.existsSync(dir) ? fs.readdirSync(dir).filter((f) => f.endsWith('.c')).sort().map((f) => path.join(dir, f)) : [];
const cRunFiles = cFilesIn(cRunDir);
const cRunTest = async (f) => {
  if (bootstrapCompiler()) return { skip: true, reason: 'the C front end is the compiler\'s' };
  const src = fs.readFileSync(f, 'utf8');
  if (native && noC && /\/\* linux-only \*\//.test(src)) return { skip: true, reason: 'links C code' };
  const flags = [...src.matchAll(/\/\* flags: (.*?) \*\//g)].flatMap((m) => m[1].trim().split(/\s+/));
  // (/* wasm-flags: ... */: more flags for a WebAssembly build)
  if (!native) flags.push(...[...src.matchAll(/\/\* wasm-flags: (.*?) \*\//g)].flatMap((m) => m[1].trim().split(/\s+/)));
  // (/* units: a.c b.c */: more translation units, next to the test)
  flags.push(...[...src.matchAll(/\/\* units: (.*?) \*\//g)].flatMap((m) => m[1].trim().split(/\s+/)).map((u) => path.join(path.dirname(f), u)));
  const errors = [...src.matchAll(/\/\* expect-error: (C\d+) \*\//g)].map((m) => m[1]);
  const warnings = [...src.matchAll(/\/\* expect-warning: (C\d+) \*\//g)].map((m) => m[1]);
  const expect = [...src.matchAll(/^\/\* expect: ?(.*?) \*\/$/gm)].map((m) => m[1]);
  if ((native && /\/\* wasm-only \*\//.test(src)) || (!native && /\/\* native-only \*\//.test(src))) return { skip: true, reason: native ? 'wasm only' : 'native only' };
  // (/* not-windows */: a rule of the other ABIs, whose Windows counterpart is another test)
  if (native && /windows/.test(process.env.AXIOM_NATIVE_TARGET || '') && /\/\* not-windows \*\//.test(src)) return { skip: true, reason: 'not for Windows' };
  // (/* windows-only */: natively for Windows targets only)
  if (/\/\* windows-only \*\//.test(src) && !(native && /windows/.test(process.env.AXIOM_NATIVE_TARGET || ''))) return { skip: true, reason: 'Windows only' };
  const out = path.join(tmp, `c-${path.basename(f, '.c')}${native ? '' : '.wasm'}`);
  if (/\/\* check-only \*\//.test(src)) {
    const c = await spawnAsync(compiler, ['check', f, `-I${path.join(root, 'cmd/axiom/internal/tests/c')}`, ...flags], { encoding: 'utf8', timeout: compileTimeout });
    return c.status === 0 ? null : `the check failed:\n${c.stdout}${c.stderr}`;
  }
  // (--units is for Axiom code: the suite run with it builds the C tests as always)
  // (a test's own optimization level wins over the suite's: tail_calls_deep.c
  // needs -O2's sibling calls)
  const pinnedO = flags.some((x) => /^-O[0-3]$/.test(x));
  const args = ['build', f, '--format=agent-json', `-I${path.join(root, 'cmd/axiom/internal/tests/c')}`, '-o', out, ...flags,
    ...extraFlags.filter((a) => !a.startsWith('--units=') && !(pinnedO && /^-O[0-3]$/.test(a)))];
  // (an expected error of an explicit --target's rules: checked for that target)
  const pinned = errors.length && flags.some((x) => x.startsWith('--target='));
  if (native && noC && !pinned) args.push(nativeTarget, path.join(root, 'cmd/axiom/internal/tests/c/host_c.c'));
  // (host_native.c is the system C library's client: the program links that
  // library, not the compiler's, whose names would take the place of its)
  else if (native && !pinned) args.push(nativeTarget, `--link=${path.join(root, 'cmd/axiom/internal/tests/c/host_native.c')}`, '--c-freestanding');
  else if (process.env.AXIOM_C_TARGET && !flags.some((x) => x.startsWith('--target='))) args.push(`--target=${process.env.AXIOM_C_TARGET}`);
  const c = await spawnAsync(compiler, args, { encoding: 'utf8', timeout: compileTimeout });
  if (c.error && c.error.code === 'ETIMEDOUT') return `the compiler did not finish in ${compileTimeout / 1000} s`;
  let json = null;
  try { json = JSON.parse(c.stdout); } catch { json = null; }
  if (errors.length) {
    const codes = json ? json.diagnostics.filter((x) => x.severity === 'error').map((x) => x.code) : [];
    if (c.status === 0) return `expected errors ${errors.join(', ')}, but compilation succeeded`;
    for (const e of errors) if (!codes.includes(e)) return `expected ${e}, got [${codes.join(', ')}]:\n${c.stdout}${c.stderr}`;
    return null;
  }
  if (c.status !== 0) return `compilation failed:\n${c.stdout}${c.stderr}`;
  const wcodes = json ? json.diagnostics.filter((x) => x.severity === 'warning').map((x) => x.code) : [];
  for (const w of warnings) if (!wcodes.includes(w)) return `expected warning ${w}, got [${wcodes.join(', ')}]`;
  if (buildOnly) return checkBuilt(out);
  const r = native ? await runNative(out) : await runWasm(out);
  let problem = r.trapped ? `unexpected trap: ${r.msg}` : null;
  // (the C library of Windows writes lines ending in \r\n)
  const want = expect.join('\n'), got = r.out.map((l) => l.replace(/\r$/, '')).join('\n');
  if (!problem && want !== got) problem = `output mismatch\n--- expected\n${want}\n--- got\n${got}`;
  return { problem, stderr: r.stderr };
};
await section('', byFile(cRunFiles, '[c]', cRunTest), { parallel: true, buildOnly: true, note: buildNote });
await section('', byFile(cFilesIn(path.join(root, 'cmd/axiom/internal/tests/c/prog')), '[c-prog]', cRunTest), { parallel: true, buildOnly: true, note: buildNote });

// C and Axiom in one program (docs/c.md): each directory of
// cmd/axiom/internal/tests/c/mix/ is a test. Its main.ax (an Axiom program
// with C units) or main.c (a C program with Axiom modules) is built with the
// directory's other .c files (and, for main.c, its .ax files), sorted, then
// those its `uses:` directive names, at the suite's optimization level for
// wasm32 (AXIOM_C_TARGET=wasm64: wasm64) or with --native for the machine,
// with no system C compiler or library: the tests print through
// c/mix/print.ax. Directives of the main file, in // or /* */ comments:
// expect: <line>, expect-error: <code> (the build fails with it),
// expect-stderr: <text> (with expect-error: the compiler's stderr has it),
// expect-status: <n> (the exit status; default 0), flags:, uses: <files>,
// wasm-only, native-only, linux-only (natively, Linux only), windows-only
// (AXIOM_NATIVE_TARGET=x86_64-windows), x86-64-only (not for an AArch64 target),
// axiom-test and axiom-profile (the program is run with that command, whose
// report must have the expect-report: lines). Programs run in
// a process of their own (on WebAssembly with the node host): they may exit.
const cMixDir = path.join(root, 'cmd/axiom/internal/tests/c/mix');
const cMixDirs = fs.existsSync(cMixDir) ? fs.readdirSync(cMixDir).filter((n) => fs.statSync(path.join(cMixDir, n)).isDirectory()).sort().map((n) => path.join(cMixDir, n)) : [];
const cMixTest = async (dir) => {
  if (bootstrapCompiler()) return { skip: true, reason: 'the C front end is the compiler\'s' };
  const axMain = fs.existsSync(path.join(dir, 'main.ax'));
  const main = path.join(dir, axMain ? 'main.ax' : 'main.c');
  const src = fs.readFileSync(main, 'utf8');
  const dv = (name) => [...src.matchAll(new RegExp(`(?:// |/\\* )${name}: ?(.*?)(?: \\*/)?$`, 'gm'))].map((m) => m[1]);
  const has = (name) => new RegExp(`(?:// |/\\* )${name}\\b`, 'm').test(src);
  if ((native && has('wasm-only')) || (!native && has('native-only'))) return { skip: true, reason: native ? 'wasm only' : 'native only' };
  if (native && has('linux-only') && (noC || process.platform !== 'linux')) return { skip: true, reason: 'Linux only' };
  if (!native && has('linux-only')) return { skip: true, reason: 'Linux only' };
  if (has('windows-only') && !(native && /windows/.test(process.env.AXIOM_NATIVE_TARGET || ''))) return { skip: true, reason: 'Windows only' };
  // (the node host's CLI does not run std/io programs for wasm64)
  if (!native && process.env.AXIOM_C_TARGET === 'wasm64') return { skip: true, reason: 'wasm32 only' };
  if (has('x86-64-only') && native && /aarch64/.test(process.env.AXIOM_NATIVE_TARGET || '')) return { skip: true, reason: 'x86-64 only' };
  const others = fs.readdirSync(dir).filter((n) => n !== path.basename(main) && (n.endsWith('.c') || n.endsWith('.s') || n.endsWith('.S') || (!axMain && n.endsWith('.ax')))).sort().map((n) => path.join(dir, n));
  const uses = dv('uses').flatMap((x) => x.trim().split(/\s+/)).map((x) => path.resolve(dir, x));
  const flags = dv('flags').flatMap((x) => x.trim().split(/\s+/));
  const errors = dv('expect-error'), want = dv('expect').join('\n');
  const status = has('expect-status') ? Number(dv('expect-status')[0]) : 0;
  const out = path.join(tmp, `mix-${path.basename(dir)}${native ? '' : '.wasm'}`);
  if (has('axiom-test') || has('axiom-profile')) {
    // (axiom test or axiom profile of the program: its report must have
    // the expect-report: lines)
    if (buildOnly) return { skip: true, reason: 'runs the program' };
    const cmd = has('axiom-test') ? ['test', '--show-output'] : ['profile'];
    const targs = [...cmd, main, ...others, ...uses, ...flags, ...(native ? [nativeTarget] : [])];
    const c = await spawnAsync(compiler, targs, { encoding: 'utf8', timeout: compileTimeout + runTimeout, cwd: tmp });
    if (c.status !== 0) return `axiom ${cmd[0]} failed (status ${c.status}):\n${c.stdout}${c.stderr}`;
    for (const t of dv('expect-report')) if (!(c.stdout + c.stderr).includes(t)) return `no '${t}' in the report:\n${c.stdout}${c.stderr}`;
    return null;
  }
  const args = ['build', main, ...others, ...uses, '--format=agent-json', '-o', out, ...flags, ...extraFlags.filter((a) => !a.startsWith('--units='))];
  if (native) args.push(nativeTarget);
  else if (process.env.AXIOM_C_TARGET) args.push(`--target=${process.env.AXIOM_C_TARGET}`);
  const c = await spawnAsync(compiler, args, { encoding: 'utf8', timeout: compileTimeout });
  if (c.error && c.error.code === 'ETIMEDOUT') return `the compiler did not finish in ${compileTimeout / 1000} s`;
  if (errors.length) {
    let codes = [];
    try { codes = JSON.parse(c.stdout).diagnostics.filter((x) => x.severity === 'error').map((x) => x.code); } catch { codes = []; }
    if (c.status === 0) return `expected errors ${errors.join(', ')}, but the build succeeded`;
    for (const e of errors) if (!codes.includes(e)) return `expected ${e}, got [${codes.join(', ')}]:\n${c.stdout}${c.stderr}`;
    for (const t of dv('expect-stderr')) if (!c.stderr.includes(t)) return `no '${t}' in the compiler's stderr:\n${c.stderr}`;
    return null;
  }
  if (c.status !== 0) return `compilation failed:\n${c.stdout}${c.stderr}`;
  if (buildOnly) return checkBuilt(out);
  { const bad = checkWindowsExe(out); if (bad) return bad; }
  const r = native ? await spawnExeAsync(out, [], { encoding: 'utf8', timeout: runTimeout })
    : await spawnAsync(process.execPath, [path.join(root, 'std/internal/runtime/host.mjs'), out], { encoding: 'utf8', timeout: runTimeout });
  if (r.error && r.error.code === 'ETIMEDOUT') return `timed out after ${runTimeout / 1000} s`;
  const got = r.stdout.split('\n').map((l) => l.replace(/\r$/, ''));
  if (got[got.length - 1] === '') got.pop();
  if (r.status !== status) return { problem: `exit status ${r.status} (signal ${r.signal}), expected ${status}; output:\n${got.join('\n')}`, stderr: r.stderr };
  return { problem: want === got.join('\n') ? null : `output mismatch\n--- expected\n${want}\n--- got\n${got.join('\n')}`, stderr: r.stderr };
};
await section('', byFile(cMixDirs, '[c-mix]', cMixTest), { parallel: true, buildOnly: true, note: buildNote });

// Linking with the objects, archives and shared libraries of the platform's
// toolchain, and with the compiler's own objects (docs/c.md, "Objects and
// libraries"): each directory of cmd/axiom/internal/tests/c/link/ is a
// test, natively on Linux (x86-64, or AArch64 with AXIOM_NATIVE_TARGET=
// aarch64) where the system's C compiler for the target is installed. Its
// files: main.c or main.ax, built by the compiler with the others; sys_*.c
// (and sys_*.cpp: the system's C++ compiler, -fno-exceptions -fno-rtti),
// compiled by the system's compiler into objects given to the build;
// arch_*.c into the archive libarch.a, so_*.c into the shared library
// libso.so (-fPIC -shared), both named with -L and -l (the program runs
// with LD_LIBRARY_PATH); ours_*.c compiled by this compiler (--emit=obj,
// with ours-flags:) into objects given to the build, ourlib_*.c into the
// archive libours.a (--emit=lib, -lours), oursso_*.c into the shared
// library liboursso.so (--emit=shared --c-lib=system, -loursso; the
// program runs with LD_LIBRARY_PATH), which cc_dlopen.c, if there is one,
// compiled by the system's C compiler, loads (its path the program's
// argument) and must print the dl-expect: lines; cc_main.c, if there is one,
// compiled and linked by the system's C compiler with the ours_ objects and
// libours.a into a second program, which must print the cc-expect: lines.
// Directives of the main file, in // or /* */ comments: expect:,
// expect-error: (E or C codes), expect-stderr:, expect-status:, flags:
// (--link=x paths are the directory's), cc-variants: flags | flags | ...
// (the sys_, arch_ and so_ files are compiled with each, the test run
// once per variant; -x86_64 and -aarch64 suffixes on the directive give a
// target's own), needs: c++.
const cLinkDir = path.join(root, 'cmd/axiom/internal/tests/c/link');
const cLinkDirs = fs.existsSync(cLinkDir) ? fs.readdirSync(cLinkDir).filter((n) => fs.statSync(path.join(cLinkDir, n)).isDirectory()).sort().map((n) => path.join(cLinkDir, n)) : [];
const linkArch = (process.env.AXIOM_NATIVE_TARGET || process.arch === 'arm64' && 'aarch64' || 'x86_64').replace(/^native$/, process.arch === 'arm64' ? 'aarch64' : 'x86_64');
const hostArch = process.arch === 'arm64' ? 'aarch64' : 'x86_64';
const sysTool = (cxx) => {
  const names = linkArch === hostArch ? (cxx ? ['g++', 'c++', 'clang++'] : ['cc', 'gcc', 'clang']) : [`${linkArch}-linux-gnu-${cxx ? 'g++' : 'gcc'}`];
  for (const n of names) if (spawnSync('sh', ['-c', `command -v ${n}`]).status === 0) return n;
  return null;
};
const cLinkTest = async (dir) => {
  if (bootstrapCompiler()) return { skip: true, reason: 'the C front end is the compiler\'s' };
  if (!native || noC || process.platform !== 'linux' || !/^(x86_64|aarch64)$/.test(linkArch)) return { skip: true, reason: 'Linux natively' };
  const cc = sysTool(false);
  if (!cc) return { skip: true, reason: 'no system C compiler for the target' };
  const axMain = fs.existsSync(path.join(dir, 'main.ax'));
  const main = path.join(dir, axMain ? 'main.ax' : 'main.c');
  const src = fs.readFileSync(main, 'utf8');
  const dv = (name) => [...src.matchAll(new RegExp(`(?:// |/\\* )${name}: ?(.*?)(?: \\*/)?$`, 'gm'))].map((m) => m[1]);
  const names = fs.readdirSync(dir).sort();
  const pick = (re) => names.filter((n) => re.test(n)).map((n) => path.join(dir, n));
  const needsCxx = dv('needs').some((x) => /c\+\+/.test(x));
  const cxx = needsCxx ? sysTool(true) : null;
  if (needsCxx && !cxx) return { skip: true, reason: 'no system C++ compiler' };
  const variantsOf = (d) => dv(d).flatMap((x) => x.split('|')).map((x) => x.trim());
  let variants = [...variantsOf('cc-variants'), ...variantsOf(`cc-variants-${linkArch}`)];
  if (!variants.length) variants = ['-O2'];
  const errors = dv('expect-error'), want = dv('expect').join('\n');
  const status = dv('expect-status').length ? Number(dv('expect-status')[0]) : 0;
  const work = path.join(tmp, `link-${path.basename(dir)}`);
  fs.mkdirSync(work, { recursive: true });
  const run = async (cmd, args, what) => {
    const r = await spawnAsync(cmd, args, { encoding: 'utf8', timeout: compileTimeout, cwd: work });
    return r.status === 0 ? null : `${what} failed:\n${r.stdout}${r.stderr}`;
  };
  // this compiler's objects and archive (the same for every variant)
  const target = `--target=${linkArch}`;
  const oursFlags = dv('ours-flags').flatMap((x) => x.trim().split(/\s+/)).filter((x) => x);
  const ours = [];
  for (const f of pick(/^ours_.*\.c$/)) {
    const o = path.join(work, path.basename(f, '.c') + '.o');
    const p = await run(compiler, ['build', f, '--emit=obj', target, ...oursFlags, '-o', o], `compiling ${path.basename(f)} (--emit=obj)`);
    if (p) return p;
    ours.push(o);
  }
  const ourlib = pick(/^ourlib_.*\.c$/);
  if (ourlib.length) {
    const p = await run(compiler, ['build', ...ourlib, '--emit=lib', target, ...oursFlags, '-o', path.join(work, 'libours.a')], 'compiling the ourlib_ files (--emit=lib)');
    if (p) return p;
  }
  const oursso = pick(/^oursso_.*\.c$/);
  if (oursso.length) {
    const p = await run(compiler, ['build', ...oursso, '--emit=shared', '--c-lib=system', target, ...oursFlags, '-o', path.join(work, 'liboursso.so')], 'compiling the oursso_ files (--emit=shared)');
    if (p) return p;
  }
  for (const v of variants) {
    const vf = v.split(/\s+/).filter((x) => x);
    const objs = [];
    for (const f of pick(/^sys_.*\.(c|cpp)$/)) {
      const o = path.join(work, path.basename(f).replace(/\.\w+$/, '.o'));
      const isCxx = f.endsWith('.cpp');
      const p = await run(isCxx ? cxx : cc, ['-c', ...vf, ...(isCxx ? ['-fno-exceptions', '-fno-rtti'] : []), f, '-o', o], `${isCxx ? cxx : cc} ${v} ${path.basename(f)}`);
      if (p) return p;
      objs.push(o);
    }
    const libFlags = [];
    const arch = pick(/^arch_.*\.c$/);
    if (arch.length) {
      const aos = [];
      for (const f of arch) {
        const o = path.join(work, path.basename(f, '.c') + '.o');
        const p = await run(cc, ['-c', ...vf, f, '-o', o], `${cc} ${path.basename(f)}`);
        if (p) return p;
        aos.push(o);
      }
      fs.rmSync(path.join(work, 'libarch.a'), { force: true });
      const p = await run('ar', ['rcs', path.join(work, 'libarch.a'), ...aos], 'ar');
      if (p) return p;
      libFlags.push('-larch');
    }
    const so = pick(/^so_.*\.c$/);
    if (so.length) {
      const p = await run(cc, ['-shared', '-fPIC', '-O2', ...so, '-o', path.join(work, 'libso.so')], `${cc} -shared`);
      if (p) return p;
      libFlags.push('-lso');
    }
    if (ourlib.length) libFlags.push('-lours');
    if (oursso.length) libFlags.push('-loursso');
    const flags = dv('flags').flatMap((x) => x.trim().split(/\s+/)).filter((x) => x).map((x) => x.startsWith('--link=') ? `--link=${path.resolve(dir, x.slice(7))}` : x);
    const others = names.filter((n) => n !== path.basename(main) && /\.c$/.test(n) && !/^(sys|arch|so|ours|ourlib|oursso|cc)_/.test(n)).map((n) => path.join(dir, n));
    const out = path.join(work, 'prog');
    const args = ['build', main, ...others, ...objs, ...ours, `-L${work}`, ...libFlags, '--format=agent-json', '-o', out, target, ...flags, ...extraFlags.filter((a) => !a.startsWith('--units='))];
    const c = await spawnAsync(compiler, args, { encoding: 'utf8', timeout: compileTimeout });
    if (errors.length) {
      let codes = [];
      try { codes = JSON.parse(c.stdout).diagnostics.filter((x) => x.severity === 'error').map((x) => x.code); } catch { codes = []; }
      if (c.status === 0) return `[${v}] expected errors ${errors.join(', ')}, but the build succeeded`;
      for (const e of errors) if (!codes.includes(e)) return `[${v}] expected ${e}, got [${codes.join(', ')}]:\n${c.stdout}${c.stderr}`;
      for (const t of dv('expect-stderr')) if (!c.stderr.includes(t)) return `[${v}] no '${t}' in the compiler's stderr:\n${c.stderr}`;
      continue;
    }
    if (c.status !== 0) return `[${v}] compilation failed:\n${c.stdout}${c.stderr}`;
    const env = { ...process.env, LD_LIBRARY_PATH: work };
    const r = await spawnExeAsync(out, [], { encoding: 'utf8', timeout: runTimeout, env });
    if (r.error && r.error.code === 'ETIMEDOUT') return `[${v}] timed out after ${runTimeout / 1000} s`;
    const got = r.stdout.split('\n');
    if (got[got.length - 1] === '') got.pop();
    if (r.status !== status) return { problem: `[${v}] exit status ${r.status} (signal ${r.signal}), expected ${status}; output:\n${got.join('\n')}`, stderr: r.stderr };
    if (want !== got.join('\n')) return { problem: `[${v}] output mismatch\n--- expected\n${want}\n--- got\n${got.join('\n')}`, stderr: r.stderr };
    for (const t of dv('expect-stderr')) if (!r.stderr.includes(t)) return `[${v}] no '${t}' in the program's stderr:\n${r.stderr}`;
  }
  // the compiler's shared library loaded by a program of the system's
  // toolchain (dlopen, dlsym)
  const ccDl = path.join(dir, 'cc_dlopen.c');
  if (fs.existsSync(ccDl) && oursso.length) {
    const out3 = path.join(work, 'dl-prog');
    const p = await run(cc, [ccDl, '-o', out3, '-ldl'], `${cc} (a program that loads the compiler's shared library)`);
    if (p) return p;
    const r = await spawnExeAsync(out3, [path.join(work, 'liboursso.so')], { encoding: 'utf8', timeout: runTimeout });
    const got = r.stdout.split('\n');
    if (got[got.length - 1] === '') got.pop();
    const want3 = dv('dl-expect').join('\n');
    if (r.status !== 0 || want3 !== got.join('\n')) return { problem: `the program loading the shared library: exit status ${r.status}\n--- expected\n${want3}\n--- got\n${got.join('\n')}`, stderr: r.stderr };
  }
  // the compiler's objects linked by the system's toolchain
  const ccMain = path.join(dir, 'cc_main.c');
  if (fs.existsSync(ccMain)) {
    const out2 = path.join(work, 'cc-prog');
    const p = await run(cc, [ccMain, ...ours, ...(ourlib.length ? [`-L${work}`, '-lours'] : []), '-o', out2], `${cc} (linking the compiler's objects)`);
    if (p) return p;
    const r = await spawnExeAsync(out2, [], { encoding: 'utf8', timeout: runTimeout });
    const got = r.stdout.split('\n');
    if (got[got.length - 1] === '') got.pop();
    const want2 = dv('cc-expect').join('\n');
    if (r.status !== 0 || want2 !== got.join('\n')) return { problem: `the system toolchain's program: exit status ${r.status}\n--- expected\n${want2}\n--- got\n${got.join('\n')}`, stderr: r.stderr };
  }
  return null;
};
await section('', byFile(cLinkDirs, '[c-link]', cLinkTest), { parallel: true, buildOnly: true, note: buildNote });

// The hosted C library (docs/c.md, "The C library"): each
// cmd/axiom/internal/tests/c/lib/<part>/*.c is a hosted C program, built
// with the library (and c/mix/print.ax, which print.h declares, and the
// files of its uses: directive) at the suite's optimization level for wasm32
// (AXIOM_C_TARGET=wasm64: wasm64), or with --native for the
// machine, and run in a process of its own whose working directory is a
// scratch directory. Directives, in // or /* */ comments: expect: <line>
// (standard output, in order), expect-status: <n> (default 0),
// expect-signal: <n> (the default action of signal n ends it),
// expect-stderr: <text> (standard error has it), expect-error: <code> (the
// build fails with it), flags:, native-only, wasm-only, linux-only
// (natively on Linux), posix-only (natively on Linux or macOS),
// windows-only (natively for a Windows target), x86-64-only (not for an
// AArch64 target); env: NAME=value (in the
// program's environment); stdin:
// <line> (the lines are the standard input, else it is empty);
// merge-stderr (standard error goes to standard output's file, so that the
// order of the two shows); check-only (only checked, with axiom check:
// with expect-error: codes, it must fail with them); host-flags: <flags>
// (on WebAssembly, for the node host: --files=memory, --root=dir);
// wasm-flags: <flags> (more compiler flags on WebAssembly, such as
// --features=+exception-handling).
const cLibDir = path.join(root, 'cmd/axiom/internal/tests/c/lib');
const cLibFiles = fs.existsSync(cLibDir) ? fs.readdirSync(cLibDir).filter((n) => fs.statSync(path.join(cLibDir, n)).isDirectory()).sort()
  .flatMap((n) => cFilesIn(path.join(cLibDir, n))) : [];
const cLibTest = async (f) => {
  if (bootstrapCompiler()) return { skip: true, reason: 'the C front end is the compiler\'s' };
  const src = fs.readFileSync(f, 'utf8');
  const dv = (name) => [...src.matchAll(new RegExp(`/\\* ${name}: ?(.*?) \\*/|// ${name}: ?(.*?)$`, 'gm'))].map((m) => m[1] ?? m[2]);
  const has = (name) => new RegExp(`(?:// |/\\* )${name}\\b`, 'm').test(src);
  const nt = process.env.AXIOM_NATIVE_TARGET || '';
  if ((native && has('wasm-only')) || (!native && has('native-only'))) return { skip: true, reason: native ? 'wasm only' : 'native only' };
  if (has('linux-only') && (!native || /windows|macos|darwin/.test(nt) || process.platform !== 'linux')) return { skip: true, reason: 'Linux only' };
  if (has('posix-only') && (!native || /windows/.test(nt))) return { skip: true, reason: 'Linux and macOS only' };
  if (has('windows-only') && !(native && /windows/.test(nt))) return { skip: true, reason: 'Windows only' };
  if (has('x86-64-only') && native && /aarch64/.test(nt)) return { skip: true, reason: 'x86-64 only' };
  const flags = dv('flags').flatMap((x) => x.trim().split(/\s+/));
  if (!native) flags.push(...dv('wasm-flags').flatMap((x) => x.trim().split(/\s+/)));
  const errors = dv('expect-error');
  if (has('check-only')) {
    const c = await spawnAsync(compiler, ['check', f, '--format=agent-json', ...flags], { encoding: 'utf8', timeout: compileTimeout });
    if (!errors.length) return c.status === 0 ? null : `the check failed:\n${c.stdout}${c.stderr}`;
    let codes = [];
    try { codes = JSON.parse(c.stdout).diagnostics.filter((x) => x.severity === 'error').map((x) => x.code); } catch { codes = []; }
    for (const e of errors) if (!codes.includes(e)) return `expected ${e}, got [${codes.join(', ')}]:\n${c.stdout}${c.stderr}`;
    return null;
  }
  const status = dv('expect-status').length ? Number(dv('expect-status')[0]) : 0;
  const rel = path.relative(cLibDir, f).replace(/[\/\\]/g, '-').replace(/\.c$/, '');
  const out = path.join(tmp, `clib-${rel}${native ? '' : '.wasm'}`);
  const uses = dv('uses').flatMap((x) => x.trim().split(/\s+/)).map((x) => path.resolve(path.dirname(f), x));
  const args = ['build', f, path.join(root, 'cmd/axiom/internal/tests/c/mix/print.ax'), ...uses, '--format=agent-json',
    `-I${path.join(root, 'cmd/axiom/internal/tests/c/mix')}`, '-o', out, ...flags, ...extraFlags.filter((a) => !a.startsWith('--units='))];
  if (native) args.push(nativeTarget);
  else if (process.env.AXIOM_C_TARGET && !flags.some((x) => x.startsWith('--target='))) args.push(`--target=${process.env.AXIOM_C_TARGET}`);
  const c = await spawnAsync(compiler, args, { encoding: 'utf8', timeout: compileTimeout });
  if (c.error && c.error.code === 'ETIMEDOUT') return `the compiler did not finish in ${compileTimeout / 1000} s`;
  if (errors.length) {
    let codes = [];
    try { codes = JSON.parse(c.stdout).diagnostics.filter((x) => x.severity === 'error').map((x) => x.code); } catch { codes = []; }
    if (c.status === 0) return `expected errors ${errors.join(', ')}, but the build succeeded`;
    for (const e of errors) if (!codes.includes(e)) return `expected ${e}, got [${codes.join(', ')}]:\n${c.stdout}${c.stderr}`;
    return null;
  }
  if (c.status !== 0) return `compilation failed:\n${c.stdout}${c.stderr}`;
  if (buildOnly) return checkBuilt(out);
  { const bad = checkWindowsExe(out); if (bad) return bad; }
  const cwd = path.join(tmp, `clib-${rel}.dir`);
  fs.mkdirSync(cwd, { recursive: true });
  const stdinLines = dv('stdin');
  const input = stdinLines.length ? stdinLines.join('\n') + '\n' : '';
  const merged = has('merge-stderr');
  const outFile = path.join(cwd, 'out.txt'), errFile = path.join(cwd, 'err.txt');
  const outFd = fs.openSync(outFile, 'w'), errFd = merged ? outFd : fs.openSync(errFile, 'w');
  const hostFlags = dv('host-flags').flatMap((x) => x.trim().split(/\s+/));
  const [cmd, ...cargs] = native ? [...execWrap, out] : [process.execPath, path.join(root, 'std/internal/runtime/host.mjs'), ...hostFlags, out];
  const r = await new Promise((resolve) => {
    let p;
    // (env: NAME=value: in the program's environment)
    const env = { ...process.env, ...Object.fromEntries(dv('env').map((x) => [x.slice(0, x.indexOf('=')), x.slice(x.indexOf('=') + 1)])) };
    try { p = spawn(cmd, cargs, { cwd, env, stdio: ['pipe', outFd, errFd] }); } catch (e) { resolve({ status: null, error: e }); return; }
    const timer = setTimeout(() => { p.kill('SIGTERM'); resolve({ status: null, timedOut: true }); }, runTimeout);
    p.on('error', (e) => { clearTimeout(timer); resolve({ status: null, error: e }); });
    p.on('close', (st, signal) => { clearTimeout(timer); resolve({ status: st, signal }); });
    p.stdin.on('error', () => {});
    p.stdin.end(input);
  });
  fs.closeSync(outFd);
  if (!merged) fs.closeSync(errFd);
  if (r.timedOut) return `timed out after ${runTimeout / 1000} s`;
  const text = (file) => fs.readFileSync(file, 'utf8');
  const got = text(outFile).split('\n').map((l) => l.replace(/\r$/, ''));
  if (got[got.length - 1] === '') got.pop();
  const stderr = merged ? '' : text(errFile);
  const want = dv('expect');
  // (expect-signal: <n>: on Linux and macOS the system's default action
  // kills the process with signal n; elsewhere the library ends it with
  // status 3 for SIGABRT, 128 + n for the others: std/c/include/signal.h)
  const sig = dv('expect-signal').length ? Number(dv('expect-signal')[0]) : 0;
  const names = { 2: 'SIGINT', 4: 'SIGILL', 6: 'SIGABRT', 8: 'SIGFPE', 11: 'SIGSEGV', 15: 'SIGTERM' };
  if (sig && native && !/windows/.test(nt)) {
    if (r.signal !== names[sig]) return { problem: `ended with status ${r.status} (signal ${r.signal}), expected signal ${names[sig]}; output:\n${got.join('\n')}`, stderr };
  } else if (r.status !== (sig ? (sig === 6 ? 3 : 128 + sig) : status)) {
    return { problem: `exit status ${r.status} (signal ${r.signal}), expected ${sig ? (sig === 6 ? 3 : 128 + sig) : status}; output:\n${got.join('\n')}`, stderr };
  }
  for (const t of dv('expect-stderr')) if (!stderr.includes(t)) return { problem: `no '${t}' in the program's stderr`, stderr };
  return { problem: want.join('\n') === got.join('\n') ? null : `output mismatch\n--- expected\n${want.join('\n')}\n--- got\n${got.join('\n')}`, stderr: dv('expect-stderr').length ? '' : stderr };
};
await section('', byFile(cLibFiles, '[c-lib]', cLibTest), { parallel: true, buildOnly: true, note: buildNote });
// The C library's index (std/c/lib/index: which unit defines each name, for
// loading the library on demand) is what cmd/internal/tools/gen_c_lib_index.mjs
// writes from the sources.
await section('', {
  'C library: std/c/lib/index is current': async () => {
    if (bootstrapCompiler()) return { skip: true, reason: 'the C front end is the compiler\'s' };
    const r = await spawnAsync(process.execPath, [path.join(root, 'cmd/internal/tools/gen_c_lib_index.mjs'), '--check'],
      { encoding: 'utf8', env: { ...process.env, AXIOM: compiler }, timeout: compileTimeout });
    return r.status === 0 ? null : `${r.stdout}${r.stderr}`;
  },
  // (every header in every --c-std declares and defines only its
  // standard's names and those reserved to it, and all the names its
  // standard requires: tools/c_header_names.mjs, c/headers/names.txt)
  'C library: the headers\' names are their standards\'': async () => {
    if (bootstrapCompiler()) return { skip: true, reason: 'the C front end is the compiler\'s' };
    const r = await spawnAsync(process.execPath, [path.join(root, 'cmd/axiom/internal/tests/tools/c_header_names.mjs')],
      { encoding: 'utf8', env: { ...process.env, AXIOM: compiler }, timeout: compileTimeout });
    return r.status === 0 ? null : `${r.stdout}${r.stderr}`;
  },
  // (every Constraints paragraph of clause 6 violated, and diagnosed:
  // tools/c_constraints.mjs over c/constraints.txt; docs/c-conformance.md)
  'C conformance: every constraint is diagnosed': async () => {
    if (bootstrapCompiler()) return { skip: true, reason: 'the C front end is the compiler\'s' };
    const r = await spawnAsync(process.execPath, [path.join(root, 'cmd/axiom/internal/tests/tools/c_constraints.mjs')],
      { encoding: 'utf8', env: { ...process.env, AXIOM: compiler }, timeout: 4 * compileTimeout });
    return r.status === 0 ? null : `${r.stdout}${r.stderr}`;
  },
  // (every translation limit of 5.2.4.1 reached, and exceeded: tools/c_limits.mjs)
  'C conformance: the translation limits': async () => {
    if (bootstrapCompiler()) return { skip: true, reason: 'the C front end is the compiler\'s' };
    const r = await spawnAsync(process.execPath, [path.join(root, 'cmd/axiom/internal/tests/tools/c_limits.mjs')],
      { encoding: 'utf8', env: { ...process.env, AXIOM: compiler }, timeout: 10 * compileTimeout });
    return r.status === 0 ? null : `${r.stdout}${r.stderr}`;
  },
});

// The C driver (main.ax: c_option, c_options, check_c_options, build_c;
// c/cc.ax): every option, several translation units, and the errors of
// options and files. The sources are written into a directory of their own.
function cDriverCases() {
  const dir = path.join(tmp, 'c-driver');
  const write = (files) => {
    fs.mkdirSync(dir, { recursive: true });
    for (const [n, t] of Object.entries(files)) {
      fs.mkdirSync(path.dirname(path.join(dir, n)), { recursive: true });
      fs.writeFileSync(path.join(dir, n), t);
    }
  };
  const ax = (args) => spawnAsync(compiler, args, { encoding: 'utf8', cwd: dir, timeout: compileTimeout });
  const fatal = async (args, re) => {
    const c = await ax(args);
    return c.status === 2 && re.test(c.stderr) ? null : `${args.join(' ')}: status ${c.status}, expected ${re}:\n${c.stdout}${c.stderr}`;
  };
  const codes = (c) => { try { return JSON.parse(c.stdout).diagnostics.map((x) => `${x.severity[0]}:${x.code}`); } catch { return null; } };
  const skip = () => (bootstrapCompiler() ? { skip: true, reason: 'the C front end is the compiler\'s' } : null);
  return {
    'C: --c-std= names': async () => {
      if (skip()) return skip();
      write({ 'std.c': 'long v = __STDC_VERSION__;\n' });
      const want = { c89: '__STDC_VERSION__', c90: '__STDC_VERSION__', c95: '199409L', c99: '199901L', c11: '201112L',
        c17: '201710L', c18: '201710L', c23: '202311L', gnu89: '__STDC_VERSION__', gnu90: '__STDC_VERSION__',
        gnu99: '199901L', gnu11: '201112L', gnu17: '201710L', gnu18: '201710L', gnu23: '202311L' };
      for (const [n, v] of Object.entries(want)) {
        const c = await ax(['build', 'std.c', '--emit=c-pp', `--c-std=${n}`]);
        if (c.status !== 0 || c.stdout.trim() !== `long v = ${v};`) return `--c-std=${n}: ${c.stdout}${c.stderr}`;
      }
      const d = await ax(['build', 'std.c', '--emit=c-pp']);
      if (d.stdout.trim() !== 'long v = 201710L;') return `the default is not c17: ${d.stdout}`;
      for (const bad of ['--c-std=gnu95', '--c-std=GNU99', '--c-char=yes', '--c-int=8', '--c-long=16', '--c-long-double=96']) {
        const p = await fatal(['build', 'std.c', bad], /E9001/);
        if (p) return p;
      }
      // (long double's formats where the target's ABIs have them: the x87's on x86-64 Linux only)
      for (const [t, bad] of [['wasm32', '80'], ['aarch64', '80'], ['x86_64-windows', '128'], ['aarch64-macos', '80']]) {
        const p = await fatal(['build', 'std.c', `--target=${t}`, `--c-long-double=${bad}`], /--c-long-double=\d+ is not supported for --target=/);
        if (p) return p;
      }
      return fatal(['build', 'std.c', '--c-int=64', '--c-long=32'], /--c-long must be at least --c-int/);
    },
    'C: -D, -U and -I, in order': async () => {
      if (skip()) return skip();
      write({ 'm.c': 'A B C f(3) E\n#ifdef Z\nz\n#endif\n#include <h.h>\n#include "h.h"\n#include <g.h>\n',
        'i1/h.h': 'one\n', 'i2/h.h': 'two\n', 'i2/g.h': 'three\n', 'h.h': 'local\n' });
      const lines = (c) => c.stdout.split('\n').filter((l) => l.trim()).join('|');
      let c = await ax(['build', 'm.c', '--emit=c-pp', '-DA', '-DB=2', '-Df(x)=x+1', '-DC=', '-DZ', '-UZ', '-Ii1', '-Ii2']);
      if (lines(c) !== '1 2 3+1 E|one|local|three') return `-D -U -I: ${c.stdout}${c.stderr}`;
      c = await ax(['build', 'm.c', '--emit=c-pp', '-D', 'E=5', '-U', 'E', '-DE=6', '-I', 'i2', '-Ii1', '-DZ']);
      if (lines(c) !== 'A B C f(3) 6|z|two|local|three') return `-D NAME, -U NAME, -I dir: ${c.stdout}${c.stderr}`;
      c = await ax(['build', 'm.c', '--emit=c-pp', '-D3x', '--format=agent-json']);
      if (c.status !== 1 || !(codes(c) || []).includes('e:C1052')) return `-D3x: ${c.stdout}${c.stderr}`;
      let p = await fatal(['build', 'm.c', '-Inope'], /include directory 'nope' \(-I\) does not exist/);
      if (p) return p;
      p = await fatal(['build', 'm.c', '-Ih.h'], /is not a directory/);
      return p || fatal(['build', 'm.c', '-D'], /'-D' needs a value/);
    },
    'C: several translation units': async () => {
      if (skip()) return skip();
      write({
        'a.c': '#include "host.h"\nstatic int helper(int x) { return x * 2; }\nint counter;\nextern int shared;\n' +
          'int b_func(int);\nint tentative;\nint main(void) {\n    counter = 5;\n' +
          '    print_i64(helper(b_func(1)) + shared + tentative);\n    return 0;\n}\n',
        'b.c': 'static int helper(int x) { return x + 100; }\nextern int counter;\nint shared = 7;\n' +
          'int b_func(int y) { return helper(y) + counter; }\n',
        'c.c': 'int counter = 3;\nint b_func(int y) { return y; }\n',
        'lib.c': 'int lib_f(int x) { return x + 1; }\nstatic int g;\n',
      });
      const out = path.join(dir, 'ab.wasm');
      const inc = `-I${path.join(root, 'cmd/axiom/internal/tests/c')}`;
      let c = await ax(['build', 'a.c', 'b.c', inc, '-o', out]);
      if (c.status !== 0) return `a.c b.c: ${c.stdout}${c.stderr}`;
      const r = await runWasm(out);
      // (each unit's helper is its own; counter, shared and tentative are one object each)
      if (r.trapped || r.out.join('|') !== '219') return `a.c b.c printed ${r.out.join('|')} ${r.msg || ''}`;
      c = await ax(['build', 'a.c', 'b.c', 'c.c', inc, '--format=agent-json']);
      if (c.status !== 1 || (codes(c) || []).join(' ') !== 'e:C3064 e:C3064') return `a.c b.c c.c: ${c.stdout}${c.stderr}`;
      c = await ax(['check', 'a.c', 'b.c', inc]);
      if (c.status !== 0) return `check a.c b.c: ${c.stdout}${c.stderr}`;
      c = await ax(['build', 'lib.c', '--format=agent-json']);
      if (c.status !== 1 || (codes(c) || []).join(' ') !== 'e:C3063') return `lib.c without main: ${c.stdout}${c.stderr}`;
      c = await ax(['build', 'lib.c', '--emit=obj', '-o', path.join(dir, 'lib.o')]);
      if (c.status !== 0 || !fs.statSync(path.join(dir, 'lib.o')).size) return `lib.c --emit=obj: ${c.stdout}${c.stderr}`;
      c = await ax(['build', 'lib.c', '--emit=ir', '-o', path.join(dir, 'lib.ir')]);
      const ir = c.status === 0 ? fs.readFileSync(path.join(dir, 'lib.ir'), 'utf8') : '';
      if (!ir.includes('@lib_f(') || ir.includes('__axiom_c_main')) return `lib.c --emit=ir: the units' code alone expected:\n${c.stderr}`;
      return fatal(['build', 'a.c', 'b.c', '--emit=c-pp'], /--emit=c-pp takes one C source/);
    },
    'C: options for Axiom code, C options without C sources': async () => {
      if (skip()) return skip();
      write({ 'w.c': 'int main(void) { return 0; }\n', 'x.ax': 'export fn main() i32 {\n    return 0;\n}\n' });
      for (const o of ['--overflow=trap', '--sanitize=address', '--coverage=afl', '--emit-header=w.h', '--units=module']) {
        const p = await fatal(['build', 'w.c', o], /is an option for Axiom code, and not for C sources/);
        if (p) return p;
      }
      for (const o of ['--c-std=c99', '--c-char=signed', '--c-int=16', '--c-long=32', '-DX', '-UX']) {
        const p = await fatal(['build', 'x.ax', o], /is an option for C sources, and the input is not one/);
        if (p) return p;
      }
      // (both have a main: a C program's Axiom module exports main, an
      // Axiom program's C unit defines one)
      for (const [args, want] of [[['w.c', 'x.ax'], 'e:E0802'], [['x.ax', 'w.c'], 'e:C3066'], [['w.c', '--link=x.ax'], 'e:E0802']]) {
        const c = await ax(['build', ...args, '--format=agent-json']);
        if (c.status !== 1 || !(codes(c) || []).includes(want)) return `${args.join(' ')}: ${c.stdout}${c.stderr}`;
      }
      for (const cmd of ['test', 'profile']) {
        const p = await fatal([cmd, 'w.c'], new RegExp(`axiom ${cmd} does not take a C source`));
        if (p) return p;
      }
      const p = await fatal(['ide', 'w.c', '--query=diagnostics'], /axiom ide answers about Axiom sources/);
      return p || fatal(['fmt', 'w.c'], /axiom fmt formats Axiom sources, and 'w.c' is a C source/);
    },
    'C: files, outputs, --emit and -O, warnings and --format=agent-json': async () => {
      if (skip()) return skip();
      write({ 'p.c': '#include "host.h"\nint main(void) {\n    print_i64(6 * 7);\nunused:\n    return 0;\n}\n', 'd.c/x': '' });
      const inc = `-I${path.join(root, 'cmd/axiom/internal/tests/c')}`;
      let p = await fatal(['build', 'nosuch.c'], /E0211.*cannot read 'nosuch.c'/);
      if (!p) p = await fatal(['build', 'p.c', 'nosuch.c', inc], /E0211.*cannot read 'nosuch.c'/);
      if (!p) p = await fatal(['build', 'p.c', inc, '-o', '.'], /the output '.' is a directory/);
      if (!p) p = await fatal(['build', 'p.c', inc, '--emit=asm'], /--emit=asm is not supported/);
      if (!p) p = await fatal(['build', 'p.c', inc, '--emit=nothing'], /unknown --emit kind/);
      if (p) return p;
      for (const o of ['-O0', '-O1', '-O2', '-O3', '-g']) {
        const out = path.join(dir, `p${o}.wasm`);
        const c = await ax(['build', 'p.c', inc, o, '-o', out, '--format=agent-json']);
        // (a warning does not fail the build, and is in the JSON)
        if (c.status !== 0 || (codes(c) || []).join(' ') !== 'w:C4040') return `${o}: status ${c.status}: ${c.stdout}${c.stderr}`;
        const r = await runWasm(out);
        if (r.out.join('|') !== '42') return `${o}: printed ${r.out.join('|')}`;
      }
      for (const [e, want] of [['c-pp', /print_i64\(6 \* 7\)/], ['c-tokens', /^keyword int/m], ['c-ast', /^translation-unit/]]) {
        const c = await ax(['build', 'p.c', inc, `--emit=${e}`]);
        if (c.status !== 0 || !want.test(c.stdout)) return `--emit=${e}: ${c.stdout.slice(0, 200)}${c.stderr}`;
      }
      const c = await ax(['build', '--help']);
      return c.status === 0 && /--c-std=c89\|c90\|c95\|c99\|c11\|c17\|c18\|c23/.test(c.stderr) ? null : `--help: ${c.stderr}`;
    },
    'C: thread-locals and atomics in a program linked statically': async () => {
      if (skip()) return skip();
      if (!native || noC) return { skip: true, reason: 'Linux targets only' };
      // (no C library: the compiler's linker, the runtime's threads and TLS blocks; the exit status says)
      write({ 'tls.c': '#include <threads.h>\nint g[4];\n_Thread_local int *tp = &g[2];\n' +
        '_Thread_local int tv = 7;\n_Thread_local long long tz;\n_Atomic int bad, sum;\n' +
        'static int w(void *a) {\n    int id = (int)(long long)a;\n    if (tp != &g[2] || tv != 7 || tz != 0) bad++;\n' +
        '    tv = id; tz = id * 10; tp = &g[id];\n    for (int i = 0; i < 1000; i++) sum++;\n' +
        '    if (tv != id || tz != id * 10 || tp != &g[id]) bad++;\n    return 0;\n}\nint main(void) {\n    thrd_t t[3];\n' +
        '    for (int i = 0; i < 3; i++) thrd_create(&t[i], w, (void *)(long long)i);\n' +
        '    for (int i = 0; i < 3; i++) thrd_join(t[i], 0);\n' +
        '    return bad * 100 + (sum == 3000) * 10 + (tv == 7) + (tp == &g[2]) * 2;\n}\n' });
      for (const o of ['-O0', '-O2']) {
        const out = path.join(dir, `tls${o}`);
        const c = await ax(['build', 'tls.c', o, nativeTarget, '-o', out]);
        if (c.status !== 0) return `build ${o}: ${c.stdout}${c.stderr}`;
        const r = execWrap.length ? spawnSync(execWrap[0], [...execWrap.slice(1), out], { encoding: 'utf8' })
          : spawnSync(out, [], { encoding: 'utf8' });
        if (r.status !== 13) return `${o}: exit status ${r.status} (13 expected) ${r.stderr || ''}`;
      }
      return null;
    },
    'C: C and Axiom in one program: the command line': async () => {
      if (skip()) return skip();
      write({ 'm.ax': 'extern "C" {\n    fn c_f(x: i32) i32;\n}\n\nexport fn main() i32 {\n    return c_f(2) - 4;\n}\n',
        'f.c': '#ifndef K\n#error K\n#endif\nint c_f(int x) { return x * K; }\n',
        'cm.c': 'int ax_g(int);\nint main(void) { return ax_g(3) - 9; }\n',
        'g.ax': 'export fn ax_g(x: i32) i32 {\n    return x * 3;\n}\n' });
      const run = async (args) => {
        const out = path.join(dir, 'mixed.wasm');
        const c = await ax(['build', ...args, '-o', out]);
        if (c.status !== 0) return `${args.join(' ')}: ${c.stdout}${c.stderr}`;
        const r = spawnSync(process.execPath, [path.join(root, 'std/internal/runtime/host.mjs'), out], { encoding: 'utf8' });
        return r.status === 0 ? null : `${args.join(' ')}: the program exited with ${r.status}: ${r.stderr}`;
      };
      // C options apply to the C units, Axiom options to the Axiom code
      let p = await run(['m.ax', 'f.c', '-DK=2', '--c-std=c99', '--overflow=trap', '-O2']);
      if (!p) p = await run(['cm.c', 'g.ax']);
      if (!p) p = await run(['cm.c', '--link=g.ax', '-O1']);
      // the header of the Axiom program's exports is written before the C units are read
      write({ 'h.ax': 'extern "C" {\n    fn c_h() i32;\n}\n\nexport fn ax_h(x: i32) i32 {\n    return x + 1;\n}\n\n' +
        'export fn main() i32 {\n    return c_h() - 8;\n}\n', 'h.c': '#include "gen/h.h"\nint c_h(void) { return ax_h(7); }\n' });
      fs.mkdirSync(path.join(dir, 'gen'), { recursive: true });
      if (!p) p = await run(['h.ax', 'h.c', '--emit-header=gen/h.h', '--c-std=c11']);
      if (p) return p;
      let c = await ax(['check', 'm.ax', 'f.c', '-DK=1']);
      if (c.status !== 0) return `check m.ax f.c: ${c.stdout}${c.stderr}`;
      c = await ax(['check', 'm.ax', 'f.c']);
      if (c.status !== 1 || !/#error/.test(c.stderr)) return `check without -DK: ${c.stdout}${c.stderr}`;
      // (axiom test takes C units: c/mix/test_with_c; axiom ide does not)
      p = await fatal(['ide', 'm.ax', 'f.c', '-DK=2'], /axiom ide does not take C sources/);
      if (!p) p = await fatal(['build', 'm.ax', 'f.c', '--emit=c-pp'], /takes one C source, and no Axiom source/);
      if (!p) p = await fatal(['build', 'm.ax', 'f.c', '--units=module'], /--units is an option for Axiom code/);
      if (!p) p = await fatal(['build', 'm.ax', 'g.ax'], /unexpected argument 'g.ax'/);
      return p || fatal(['build', 'cm.c', 'g.ax', '--overflow=trap'], /--overflow is an option for Axiom code/);
    },
    'C: what is linked: only what is reached': async () => {
      if (skip()) return skip();
      // a library of 200 functions and a large table, of which a program
      // calls one: the executable is the size of the program without it
      let lib = '';
      for (let i = 0; i < 200; i++) lib += `int lib_f${i}(int x) { int s = 0; for (int k = 0; k < x; k++) s += k * ${i} + (s >> 3); return s ^ ${i}; }\n`;
      lib += 'static char lib_table[50000] = {1};\nint lib_t(int i) { return lib_table[i]; }\n';
      write({ 'lib.c': lib, 'hello.c': 'int lib_f7(int);\nint main(void) { return lib_f7(3) & 1; }\n',
        'alone.c': 'static int lib_f7(int x) { int s = 0; for (int k = 0; k < x; k++) s += k * 7 + (s >> 3); return s ^ 7; }\n' +
          'int main(void) { return lib_f7(3) & 1; }\n',
        'ax.ax': 'extern "C" {\n    fn lib_f7(x: i32) i32;\n}\n\nexport fn main() i32 {\n    return lib_f7(3) & 1;\n}\n' });
      const targets = ['wasm32', 'x86_64', 'aarch64', 'x86_64-windows', 'aarch64-macos'];
      for (const t of targets) {
        for (const o of ['-O0', '-O2']) {
          const size = async (args) => {
            const out = path.join(dir, `size-${t}${o}`);
            const c = await ax(['build', ...args, `--target=${t}`, o, '--pdb=off', '-o', out]);
            return c.status === 0 ? fs.statSync(out).size : `${args.join(' ')} --target=${t} ${o}: ${c.stdout}${c.stderr}`;
          };
          const a = await size(['hello.c', 'lib.c']), b = await size(['alone.c']), x = await size(['ax.ax', 'lib.c']);
          for (const v of [a, b, x]) if (typeof v === 'string') return v;
          // (the program's own code is the same, but for its symbol names and a few bytes of alignment)
          if (a > b + 1024) return `--target=${t} ${o}: ${a} bytes with the library, ${b} without`;
          if (x > a + 1024 * 64) return `--target=${t} ${o}: an Axiom program with the library: ${x} bytes`;
        }
      }
      // an object keeps every definition
      const obj = path.join(dir, 'lib.o');
      const c = await ax(['build', 'lib.c', '--emit=obj', '--target=x86_64', '-o', obj]);
      if (c.status !== 0 || !fs.readFileSync(obj).includes('lib_f199')) return `lib.c --emit=obj: ${c.stdout}${c.stderr}`;
      return null;
    },
    'C: the C library directories (std/c/lib, std/c/rt)': async () => {
      if (skip()) return skip();
      // a standard library whose c/lib and c/rt are this test's (the rest
      // is the real one's, linked)
      const std = path.join(dir, 'fakestd');
      fs.rmSync(std, { recursive: true, force: true });
      fs.mkdirSync(path.join(std, 'c/lib'), { recursive: true });
      fs.mkdirSync(path.join(std, 'c/rt'), { recursive: true });
      for (const n of fs.readdirSync(path.join(root, 'std'))) if (n !== 'c') fs.symlinkSync(path.join(root, 'std', n), path.join(std, n));
      fs.symlinkSync(path.join(root, 'std/c/include'), path.join(std, 'c/include'));
      fs.writeFileSync(path.join(std, 'c/lib/a.c'), 'int rt_twice(int);\nint lib_used(int x) { return rt_twice(x) + 1; }\n' +
        'int lib_replaced(int x) { return -1; }\nint rt_unused_fn(int);\nint lib_unused(int x) { return rt_unused_fn(x); }\n');
      fs.writeFileSync(path.join(std, 'c/rt/rt.ax'), 'export fn rt_twice(x: i32) i32 {\n    return x * 2;\n}\n\n' +
        'export fn rt_unused_fn(x: i32) i32 {\n    return x;\n}\n');
      write({ 'use.c': 'int lib_used(int);\nint lib_replaced(int x) { return x + 100; }\n' +
        'int main(void) { return lib_used(20) + lib_replaced(0) - 141; }\n' });
      const out = path.join(dir, 'use.wasm');
      let c = await ax(['build', 'use.c', `--std=${std}`, '-o', out]);
      if (c.status !== 0) return `use.c with the library: ${c.stdout}${c.stderr}`;
      let r = spawnSync(process.execPath, [path.join(root, 'std/internal/runtime/host.mjs'), out], { encoding: 'utf8' });
      if (r.status !== 0) return `use.c: exit ${r.status} ${r.stderr}`;
      if (fs.readFileSync(out).includes('rt_unused_fn')) return 'an unused export of std/c/rt is in the program';
      // without the library, lib_used is the host's (WebAssembly imports it)
      c = await ax(['build', 'use.c', `--std=${std}`, '--c-freestanding', '-o', out]);
      if (c.status !== 0) return `--c-freestanding: ${c.stdout}${c.stderr}`;
      const imps = WebAssembly.Module.imports(new WebAssembly.Module(fs.readFileSync(out))).map((i) => `${i.module}.${i.name}`);
      if (!imps.includes('env.lib_used')) return `--c-freestanding: imports ${imps.join(', ')}`;
      return null;
    },
    'C: a package with C sources (c-sources)': async () => {
      if (skip()) return skip();
      write({ 'pk/axiom.toml': '[package]\nname = "pk"\nversion = "0.1.0"\nc-sources = "csrc"\n',
        'pk/src/main.ax': 'extern "C" {\n    fn c_sq(x: i32) i32;\n}\n\nexport fn ax_one() i32 {\n    return 1;\n}\n\n' +
          'export fn main() i32 {\n    return c_sq(5) - 26;\n}\n',
        'pk/csrc/sq.c': '#include "inc/k.h"\nint c_sq(int x) { return x * x + ONE(); }\n',
        'pk/csrc/inc/k.h': 'int ax_one(void);\n#define ONE() ax_one()\n',
        'bad/axiom.toml': '[package]\nname = "bad"\nversion = "0.1.0"\nc-sources = "nope"\n', 'bad/src/main.ax': '' });
      const out = path.join(dir, 'pk.wasm');
      const c = await ax(['build', 'pk', '-o', out]);
      if (c.status !== 0) return `the package: ${c.stdout}${c.stderr}`;
      const r = spawnSync(process.execPath, [path.join(root, 'std/internal/runtime/host.mjs'), out], { encoding: 'utf8' });
      if (r.status !== 0) return `the package's program: exit ${r.status} ${r.stderr}`;
      const b = await ax(['build', 'bad']);
      return b.status === 1 && /c-sources = "\.\.\." must name a directory/.test(b.stderr) ? null : `bad: ${b.status} ${b.stderr}`;
    },
    'C: C and Axiom built for Windows and macOS': async () => {
      if (skip()) return skip();
      // the c/mix programs, built (not run: that needs the systems)
      for (const d of cMixDirs) {
        const axMain = fs.existsSync(path.join(d, 'main.ax'));
        const main = path.join(d, axMain ? 'main.ax' : 'main.c');
        const src = fs.readFileSync(main, 'utf8');
        if (/expect-error|linux-only|windows-only/.test(src)) continue;
        const others = fs.readdirSync(d).filter((n) => n !== path.basename(main) && (n.endsWith('.c') || n.endsWith('.s') || n.endsWith('.S') || (!axMain && n.endsWith('.ax')))).map((n) => path.join(d, n));
        const uses = [...src.matchAll(/(?:\/\/ |\/\* )uses: (.*?)(?: \*\/)?$/gm)].flatMap((m) => m[1].trim().split(/\s+/)).map((x) => path.resolve(d, x));
        const flags = [...src.matchAll(/(?:\/\/ |\/\* )flags: (.*?)(?: \*\/)?$/gm)].flatMap((m) => m[1].trim().split(/\s+/));
        for (const t of ['aarch64-macos', 'x86_64-windows']) {
          const out = path.join(dir, `mix-${path.basename(d)}-${t}`);
          const c = await ax(['build', main, ...others, ...uses, ...flags, `--target=${t}`, '--pdb=off', '-o', out]);
          if (c.status !== 0) return `${path.basename(d)} for ${t}: status ${c.status} ${c.signal || ""} ${c.error || ""} ${c.stdout}${c.stderr}`;
          const bytes = fs.readFileSync(out);
          if (t === 'aarch64-macos') {
            const m = checkMachO(bytes);
            if (m.errs.length) return `${path.basename(d)}: ${m.errs.join('\n')}`;
          } else if (bytes.readUInt16LE(0) !== 0x5a4d) return `${path.basename(d)}: not a PE file`;
        }
      }
      return null;
    },
    'C: Windows: data of a DLL, and unwinding with a variable length array': async () => {
      if (skip()) return skip();
      // (ntdll.dll's objects: the system's C library, which has others, is not linked)
      write({ 'dll.c': 'extern unsigned short NlsAnsiCodePage;\nextern unsigned char NlsMbCodePageTag;\nint get(void) { return NlsAnsiCodePage; }\n' +
          'unsigned short *where(void) { return &NlsAnsiCodePage; }\nint main(void) { return (get() == *where()) + 2 * NlsMbCodePageTag; }\n',
        'dllinit.c': 'extern unsigned short NlsAnsiCodePage;\nunsigned short *p = &NlsAnsiCodePage;\nint main(void) { return *p; }\n',
        'vla.c': 'int use(volatile int *p, int n);\ndouble dbl(double *p);\n' +
          'int f(int n, double x) {\n    int a[n];\n    double y = x * 2;\n    for (int i = 0; i < n; i++) a[i] = i;\n' +
          '    int r = use(a, n);\n    int b[n + r];\n    b[0] = r;\n    return r + (int)(dbl(&y) + x) + use(b, 1);\n}\n' +
          'int use(volatile int *p, int n) { int s = 0; for (int i = 0; i < n; i++) s += p[i]; return s; }\n' +
          'double dbl(double *p) { return *p * 2; }\nint main(void) { return f(10, 1.5) & 0xff; }\n' });
      const out = path.join(dir, 'dll.exe');
      let c = await ax(['build', 'dll.c', '--target=x86_64-windows', '--pdb=off', '-O2', '-o', out]);
      if (c.status !== 0) return `dll.c: ${c.stdout}${c.stderr}`;
      const pe = peImports(fs.readFileSync(out));
      for (const n of ['NlsAnsiCodePage', 'NlsMbCodePageTag']) if (!(pe['ntdll.dll'] || []).includes(n)) return `dll.c: no import of ${n}: ${JSON.stringify(pe)}`;
      if (pe['msvcrt.dll']) return `dll.c: imports from msvcrt.dll: ${JSON.stringify(pe)}`;
      c = await ax(['build', 'dllinit.c', '--target=x86_64-windows', '--format=agent-json', '-o', out]);
      if (c.status !== 1 || !(codes(c) || []).includes('e:C3067')) return `dllinit.c: ${c.stdout}${c.stderr}`;
      for (const o of ['-O0', '-O2']) {
        c = await ax(['build', 'vla.c', '--target=x86_64-windows', '--pdb=off', o, '-o', out]);
        if (c.status !== 0) return `vla.c ${o}: ${c.stdout}${c.stderr}`;
        const p = checkDynUnwind(fs.readFileSync(out));
        if (p) return `vla.c ${o}: ${p}`;
      }
      return null;
    },
    'C: native stack traces through C functions': async () => {
      if (skip()) return skip();
      if (!native || noC || execWrap.length || process.platform !== 'linux') return { skip: true, reason: 'run on this Linux machine only' };
      write({ 'crash.c': 'static int deref(int *p) {\n    return *p;\n}\nstatic int middle(int *p) {\n    return deref(p) + 1;\n}\n' +
        'int main(void) {\n    int *volatile q = 0;\n    return middle(q);\n}\n' });
      const out = path.join(dir, 'crash');
      const c = await ax(['build', 'crash.c', '-O0', nativeTarget, '-o', out]);
      if (c.status !== 0) return `build: ${c.stdout}${c.stderr}`;
      const r = spawnSync(out, [], { encoding: 'utf8' });
      return /at deref\n\s+at middle \(crash\.c:5:12\)\n\s+at main \(crash\.c:9:12\)/.test(r.stderr) ? null : `the trace:\n${r.stderr}`;
    },
    // At -O2 the frames that exist are all there, with the calls inlined
    // into them (their call-site records carry the inlining): here deep,
    // tailer and early are inlined into main, and the crash is in a
    // function reached by an indirect call, which names it only so.
    'C: native stack traces through C functions at -O2': async () => {
      if (skip()) return skip();
      if (!native || noC || execWrap.length || process.platform !== 'linux') return { skip: true, reason: 'run on this Linux machine only' };
      write({ 'crash2.c': 'static int *volatile gp;\nstatic volatile int sink;\nint leafcrash(int x) {\n    return *gp + x;\n}\n' +
        'static int (*volatile leafp)(int) = leafcrash;\nint early(int *p, int x) {\n    if (!p) {\n        return leafp(x) + 1;\n' +
        '    }\n    return *p;\n}\nint tailer(int x) {\n    sink = x;\n    return early(0, x + 1);\n}\n' +
        'int deep(int n) {\n    return n + tailer(n);\n}\nint main(int argc, char **argv) {\n    (void)argv;\n' +
        '    return deep(argc + 2);\n}\n' });
      const out = path.join(dir, 'crash2');
      const c = await ax(['build', 'crash2.c', '-O2', nativeTarget, '-o', out]);
      if (c.status !== 0) return `build: ${c.stdout}${c.stderr}`;
      const r = spawnSync(out, [], { encoding: 'utf8' });
      return /at <indirect call>\n\s+at early \(crash2\.c:9:16\)\n\s+at tailer \(crash2\.c:15:12\)\n\s+at deep \(crash2\.c:18:16\)\n\s+at main \(crash2\.c:22:12\)/.test(r.stderr) ? null : `the trace:\n${r.stderr}`;
    },
    // With line tables (-g, --debug-info=lines) the innermost frame shows
    // the faulting instruction's line too (the frame table's line rows:
    // cmd/axiom/src/ftab.ax), also where no call was recorded (all of the
    // program inlined into main at -O2); the code is the same.
    'C: native stack traces with the faulting line (-g)': async () => {
      if (skip()) return skip();
      if (!native || noC || execWrap.length || process.platform !== 'linux') return { skip: true, reason: 'run on this Linux machine only' };
      write({ 'crash.c': 'static int deref(int *p) {\n    return *p;\n}\nstatic int middle(int *p) {\n    return deref(p) + 1;\n}\n' +
        'int main(void) {\n    int *volatile q = 0;\n    return middle(q);\n}\n',
        'crash2.c': 'static int *volatile gp;\nstatic volatile int sink;\nint leafcrash(int x) {\n    return *gp + x;\n}\n' +
        'static int (*volatile leafp)(int) = leafcrash;\nint early(int *p, int x) {\n    if (!p) {\n        return leafp(x) + 1;\n' +
        '    }\n    return *p;\n}\nint tailer(int x) {\n    sink = x;\n    return early(0, x + 1);\n}\n' +
        'int deep(int n) {\n    return n + tailer(n);\n}\nint main(int argc, char **argv) {\n    (void)argv;\n' +
        '    return deep(argc + 2);\n}\n' });
      const out = path.join(dir, 'crash-g');
      const cases = [['crash.c', ['-O0', '-g'], /at deref \(crash\.c:2\)\n\s+at middle \(crash\.c:5:12\)\n\s+at main \(crash\.c:9:12\)/],
        ['crash.c', ['-O0', '--debug-info=lines'], /at deref \(crash\.c:2\)\n\s+at middle \(crash\.c:5:12\)/],
        ['crash.c', ['-O2', '-g'], /stack trace \(innermost first\):\n\s+at main \(crash\.c:[25]\)\n/],
        ['crash2.c', ['-O2', '-g'], /at leafcrash \(crash2\.c:4\)\n\s+at early \(crash2\.c:9:16\)/]];
      for (const [src, flags, want] of cases) {
        const c = await ax(['build', src, ...flags, nativeTarget, '-o', out]);
        if (c.status !== 0) return `build: ${c.stdout}${c.stderr}`;
        const r = spawnSync(out, [], { encoding: 'utf8' });
        if (!want.test(r.stderr)) return `${src} ${flags.join(' ')}: the trace:\n${r.stderr}`;
      }
      return null;
    },
  };
}

// A C program as an object and as IR: the start-up's module with the unit in it.
await section('C programs', {
  '--emit=ir and --emit=obj': async () => {
    if (bootstrapCompiler()) return { skip: true, reason: 'the C front end is the compiler\'s' };
    const f = path.join(cRunDir, 'funcs.c');
    const inc = `-I${path.join(root, 'cmd/axiom/internal/tests/c')}`;
    const ir = path.join(tmp, 'c-emit.ir'), obj = path.join(tmp, 'c-emit.o');
    let c = await spawnAsync(compiler, ['build', f, inc, '--emit=ir', '-o', ir, ...targetFlags], { encoding: 'utf8' });
    if (c.status !== 0) return `--emit=ir failed:\n${c.stdout}${c.stderr}`;
    const text = fs.readFileSync(ir, 'utf8');
    for (const fn of ['@__axiom_c_main(', '@axiom.c.main(', '@old(']) if (!text.includes(fn)) return `the IR has no ${fn}`;
    c = await spawnAsync(compiler, ['build', f, inc, '--emit=obj', '-o', obj, ...targetFlags], { encoding: 'utf8' });
    if (c.status !== 0) return `--emit=obj failed:\n${c.stdout}${c.stderr}`;
    return fs.statSync(obj).size > 0 ? null : 'an empty object';
  },
  // (C's attribute visibility in the ELF object's symbols: hidden and
  // internal are STV_HIDDEN, protected and default STV_DEFAULT)
  'visibility in --emit=obj': async () => {
    if (bootstrapCompiler()) return { skip: true, reason: 'the C front end is the compiler\'s' };
    const f = path.join(tmp, 'c-vis.c'), obj = path.join(tmp, 'c-vis.o');
    fs.writeFileSync(f, '__attribute__((visibility("hidden"))) int vis_h(void) { return 1; }\n' +
      '__attribute__((visibility("internal"))) int vis_i(void) { return 2; }\n' +
      '__attribute__((visibility("protected"))) int vis_p(void) { return 3; }\n' +
      'int vis_d(void) { return 4; }\n__attribute__((visibility("hidden"))) int vis_hd = 5;\nint vis_dd = 6;\n' +
      '#pragma GCC visibility push(hidden)\nint vis_ph(void) { return 7; }\n#pragma GCC visibility pop\n');
    const c = await spawnAsync(compiler, ['build', f, '--c-std=gnu17', '--emit=obj', '-o', obj, '--target=x86_64'], { encoding: 'utf8' });
    if (c.status !== 0) return `--emit=obj failed:\n${c.stdout}${c.stderr}`;
    const b = fs.readFileSync(obj);
    const shoff = Number(b.readBigUInt64LE(0x28)), shentsize = b.readUInt16LE(0x3a), shnum = b.readUInt16LE(0x3c);
    const sh = (k) => { const o = shoff + k * shentsize; return { type: b.readUInt32LE(o + 4), off: Number(b.readBigUInt64LE(o + 0x18)), size: Number(b.readBigUInt64LE(o + 0x20)), link: b.readUInt32LE(o + 0x28) }; };
    const other = new Map();
    for (let k = 0; k < shnum; k++) {
      const s = sh(k);
      if (s.type !== 2) continue;  // SHT_SYMTAB
      const str = sh(s.link);
      for (let o = s.off; o < s.off + s.size; o += 24) {
        const n = b.readUInt32LE(o), e = b.indexOf(0, str.off + n);
        other.set(b.toString('latin1', str.off + n, e), b[o + 5] & 3);
      }
    }
    const want = { vis_h: 2, vis_i: 2, vis_p: 0, vis_d: 0, vis_hd: 2, vis_dd: 0, vis_ph: 2 };
    for (const [n, v] of Object.entries(want)) if (other.get(n) !== v) return `${n}: visibility ${other.get(n)}, not ${v}`;
    return null;
  },
  'diagnostics at #line positions': async () => {
    if (bootstrapCompiler()) return { skip: true, reason: 'the C front end is the compiler\'s' };
    const f = path.join(cRunDir, 'err_line_presumed.c');
    const c = await spawnAsync(compiler, ['build', f, '-o', path.join(tmp, 'c-line')], { encoding: 'utf8' });
    for (const want of ['gen.y:100:9: error[C3001]', 'gen.y:7:9: error[C3001]']) if (!c.stderr.includes(want)) return `no ${want} in:\n${c.stderr}`;
    const j = await spawnAsync(compiler, ['build', f, '--format=agent-json', '-o', path.join(tmp, 'c-line')], { encoding: 'utf8' });
    const d = JSON.parse(j.stdout).diagnostics[0];
    return d.span.start_line === 7 && d.presumed_file === 'gen.y' && d.presumed_line === 100 ? null : `got ${JSON.stringify(d)}`;
  },
  ...cDriverCases(),
});

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
  if (buildOnly) {
    // (the test binary, built and checked: --no-run)
    const out = path.join(tmp, `test-${path.relative(root, f).replace(/[/.]/g, '-')}${/windows/.test(nativeTarget) ? '.exe' : ''}`);
    const c = await spawnAsync(compiler, ['test', f, '--format=agent-json', `--no-run=${out}`, ...testTargets, ...d.flags, ...extraFlags], { encoding: 'utf8', timeout: compileTimeout });
    if (c.status !== 0) return `the build failed (status ${c.status}):\n${c.stdout}${c.stderr}`;
    return checkBuilt(out);
  }
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
}), { parallel: true, buildOnly: true, note: buildNote });

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
    // (// stack-wasm-O0: on WebAssembly below -O2, in place of that test's stack:)
    const wasmStack = new Map([...src.matchAll(/^\/\/ stack-wasm-O0: (.+?) \| (.+)$/gm)].map((m) => [m[1], m[2]]));
    const low = !extraFlags.some((a) => a === '-O2' || a === '-O3') && !bootstrapCompiler();
    for (const [, kind, tname, want0] of src.matchAll(/^\/\/ (stack|errors): (.+?) \| (.+)$/gm)) {
      const want = !native && low && kind === 'stack' && wasmStack.has(tname) ? wasmStack.get(tname) : want0;
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
  // ax_tz_local (std/README.md): the zone that the host builds from what the
  // JavaScript engine knows of $TZ is the database's, for zones whose
  // daylight saving flags the engine's offsets show (tests/host/tz_host.ax)
  'the local zone without a database (ax_tz_local)': async () => {
    if (bootstrapCompiler()) return { skip: true, reason: 'tz_host.ax is checked with the compiler' };
    const out = path.join(tmp, 'tz_host.wasm');
    const c = spawnSync(compiler, ['build', path.join(root, 'cmd/axiom/internal/tests/host/tz_host.ax'), '--target=wasm32', '-o', out], { encoding: 'utf8' });
    if (c.status !== 0) return `build failed:\n${c.stdout}${c.stderr}`;
    for (const z of ['America/New_York', 'Europe/Paris', 'Asia/Kolkata', 'Australia/Lord_Howe', 'America/Sao_Paulo', 'Pacific/Chatham', 'Asia/Tehran', 'UTC']) {
      const r = spawnSync(process.execPath, [path.join(root, 'std/internal/runtime/host.mjs'), out, '--', z],
        { encoding: 'utf8', env: { ...process.env, TZ: z } });
      if (r.status === 3) return { skip: true, reason: 'no time zone database' };
      if (r.status !== 0 || r.stdout.trim() !== '0') return `${z}: status ${r.status}, mismatches: ${r.stdout}${r.stderr}`;
    }
    return null;
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
      // (the host's cc links for the host)
      if (process.env.AXIOM_NATIVE_TARGET) return { skip: true, reason: 'not when cross testing' };
      if (spawnSync('cc', ['--version']).status !== 0) return { skip: true, reason: 'no C compiler' };
      prepare();
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
    'compiled in parts (AXIOM_PARTS=3): runs, its trace names its functions, the same bytes each time': () => {
      if (!native || isBootstrap()) return { skip: true, reason: 'the compiler, native' };
      prepare();
      const f = path.join(dir, 'parts.ax');
      fs.writeFileSync(f, [
        'import "std/io" as io;', '',
        'let mut start: i32 = 3;', '',
        'fn deep(n: i32) i32 {', '    if n > 3 {', '        @trap();', '    }', '    return n;', '}', '',
        'fn mid(n: i32) i32 {', '    return deep(n + 1) * 2;', '}', '',
        'export fn main() i32 {',
        '    io.stdout().write_all("parts ok\\n") catch return 1;',
        '    return mid(start);', '}', ''].join('\n'));
      const env = { ...process.env, AXIOM_PARTS: '3' };
      const outs = [];
      // (to one path: a Windows executable names its PDB by its path, and
      // its PDB's GUID is a hash of the image)
      const o = path.join(dir, 'parts1');
      for (let k = 0; k < 2; k++) {
        const c = spawnSync(compiler, ['build', f, '-O2', '-o', o, nativeTarget], { encoding: 'utf8', env });
        if (c.status) return c.stderr;
        outs.push(fs.readFileSync(o));
      }
      if (!outs[0].equals(outs[1])) return 'two builds differ';
      const r = hostRun(o);
      return r.status !== 0 && /parts ok/.test(r.stdout) && /at deep/.test(r.stderr) && /at main/.test(r.stderr)
        ? null : `status ${r.status}:\n${r.stdout}${r.stderr}`;
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
    // --coverage=afl (spec §16.9): an instrumented program runs as usual, and
    // under afl-showmap (when installed) its two inputs, taking different
    // branches, cover different edges.
    '--coverage=afl: runs as usual, and afl-showmap sees the branches': () => {
      if (isBootstrap()) return { skip: true, reason: 'the compiler only' };
      if (process.platform !== 'linux' || !['x64', 'arm64'].includes(process.arch) || process.env.AXIOM_NATIVE_TARGET) {
        return { skip: true, reason: 'a Linux host, not cross testing' };
      }
      prepare();
      const f = path.join(dir, 'cov.ax');
      fs.writeFileSync(f, [
        'import "std/io" as io;', 'import "std/os" as os;', '',
        'fn classify(s: []u8) []u8 {',
        '    if (s.len > 1) and (s[0] == \'a\') and (s[1] == \'b\') {', '        return "ab";', '    }',
        '    let mut n: usize = 0;', '    for c in s {', '        if c == \'x\' {', '            n += 1;', '        }', '    }',
        '    if n > 2 {', '        return "many";', '    }', '    return "other";', '}', '',
        'export fn main() i32 {', '    let s = os.arg(1) catch return 1;',
        '    io.stdout().write_all(classify(s)) catch return 1;', '    return 0;', '}', ''].join('\n'));
      const o = path.join(dir, 'cov');
      const c = spawnSync(compiler, ['build', f, '--coverage=afl', '-o', o, '--target=native', ...extraFlags], { encoding: 'utf8' });
      if (c.status) return c.stderr;
      for (const [arg, want] of [['ab', 'ab'], ['xxxx', 'many']]) {
        const r = spawnSync(o, [arg], { encoding: 'utf8' });
        if (r.status !== 0 || r.stdout !== want) return `${arg}: status ${r.status}: ${r.stdout}${r.stderr}`;
      }
      if (spawnSync('afl-showmap', ['-h']).error) return { skip: true, reason: 'ran; no afl-showmap to check the coverage' };
      const edges = [];
      for (const arg of ['ab', 'xxxx']) {
        const m = path.join(dir, `cov-${arg}.map`);
        const r = spawnSync('afl-showmap', ['-q', '-o', m, '--', o, arg], { encoding: 'utf8', timeout: 60000 });
        if (r.status !== 0 || !fs.existsSync(m)) return `afl-showmap ${arg}: status ${r.status}: ${r.stdout}${r.stderr}`;
        edges.push(new Set(fs.readFileSync(m, 'utf8').split('\n').filter((l) => l).map((l) => l.split(':')[0])));
      }
      if (edges[0].size < 10 || edges[1].size < 10) return `few edges: ${edges[0].size}, ${edges[1].size}`;
      const only = (a, b) => [...a].filter((e) => !b.has(e)).length;
      return only(edges[0], edges[1]) && only(edges[1], edges[0]) ? null : 'both inputs cover the same edges';
    },
    '--coverage=afl for wasm32 is E9001': () => {
      if (isBootstrap()) return { skip: true, reason: 'the compiler only' };
      prepare();
      const c = spawnSync(compiler, ['build', src, '--coverage=afl', '--target=wasm32', '--format=agent-json', '-o', path.join(dir, 'x.wasm')],
        { encoding: 'utf8' });
      return c.status === 2 && /"E9001".*--coverage=afl is not supported/.test(c.stdout) ? null : `${c.status}: ${c.stdout}${c.stderr}`;
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

// axiom update, of an install from a release archive: from a mirror in a
// directory (AXIOM_UPDATE_URL), whose release 9.9.9 is a script that says so.
{
  const dir = path.join(tmp, 'update');
  const isBootstrap = () => /bootstrap/.test(spawnSync(compiler, ['version'], { encoding: 'utf8' }).stdout);
  const plat = { linux: 'linux', darwin: 'macos' }[process.platform];
  const arch = { x64: 'x86_64', arm64: 'aarch64' }[process.arch];
  const has = (tool) => spawnSync(tool, ['--version']).status === 0;
  const update = (exe, ...args) => spawnSync(exe, ['update', ...args, '--format=agent-json'], {
    encoding: 'utf8', env: { ...process.env, AXIOM_UPDATE_URL: `file://${path.join(dir, 'mirror')}` } });
  const install = () => {
    fs.rmSync(dir, { recursive: true, force: true });
    const inst = path.join(dir, 'inst');
    fs.mkdirSync(path.join(inst, 'bin'), { recursive: true });
    fs.copyFileSync(compiler, path.join(inst, 'bin/axiom'));
    fs.chmodSync(path.join(inst, 'bin/axiom'), 0o755);
    fs.symlinkSync(path.join(root, 'std'), path.join(inst, 'std'));
    fs.writeFileSync(path.join(inst, 'RELEASE-NOTES.md'), 'old\n');
    // the release
    const name = `axiom-9.9.9-${plat}-${arch}`;
    const rel = path.join(dir, 'rel', name);
    fs.mkdirSync(path.join(rel, 'bin'), { recursive: true });
    fs.mkdirSync(path.join(rel, 'std'));
    fs.writeFileSync(path.join(rel, 'bin/axiom'), '#!/bin/sh\necho "axiom 9.9.9"\n', { mode: 0o755 });
    fs.writeFileSync(path.join(rel, 'std/mem.ax'), '');
    fs.writeFileSync(path.join(rel, 'RELEASE-NOTES.md'), 'new\n');
    const down = path.join(dir, 'mirror/releases/download/v9.9.9');
    fs.mkdirSync(down, { recursive: true });
    const archive = path.join(down, `${name}.tar.gz`);
    const t = spawnSync('tar', ['-czf', archive, '-C', path.join(dir, 'rel'), name], { encoding: 'utf8' });
    if (t.status) throw new Error(`tar: ${t.stderr}`);
    const sum = crypto.createHash('sha256').update(fs.readFileSync(archive)).digest('hex');
    fs.writeFileSync(path.join(down, 'SHA256SUMS'), `${sum}  ${name}.tar.gz\n`);
    return path.join(inst, 'bin/axiom');
  };
  const skip = () => {
    if (isBootstrap()) return { skip: true, reason: 'the compiler only' };
    if (!plat || !arch || process.env.AXIOM_NATIVE_TARGET) return { skip: true, reason: 'Linux and macOS hosts' };
    if (!has('curl') || !has('tar')) return { skip: true, reason: 'no curl or tar' };
    return null;
  };
  const code = (r, status, c) => r.status === status && r.stdout.includes(`"${c}"`) ? null : `${r.status}: ${r.stdout}${r.stderr}`;
  await section('axiom update', {
    'the bootstrap compiler does not update (E9008)': () => {
      if (!isBootstrap()) return { skip: true, reason: 'the bootstrap compiler only' };
      return code(spawnSync(compiler, ['update', '--format=agent-json'], { encoding: 'utf8' }), 1, 'E9008');
    },
    'a release install refuses HEAD, in any case, and a non-version (E9001)': () => {
      const s = skip();
      if (s) return s;
      const exe = install();
      return code(update(exe, 'HEAD'), 2, 'E9001') ?? code(update(exe, 'Head'), 2, 'E9001') ??
        code(update(exe, '0.2'), 2, 'E9001');
    },
    'a release that does not exist, or does not match its checksum, changes nothing (E9008)': () => {
      const s = skip();
      if (s) return s;
      const exe = install();
      const sums = path.join(dir, 'mirror/releases/download/v9.9.9/SHA256SUMS');
      fs.writeFileSync(sums, fs.readFileSync(sums, 'utf8').replace(/^./, (c) => (c === '0' ? '1' : '0')));
      const e = code(update(exe, '9.9.8'), 1, 'E9008') ?? code(update(exe, 'v9.9.9'), 1, 'E9008');
      if (e) return e;
      const v = spawnSync(exe, ['version'], { encoding: 'utf8' }).stdout;
      return v.startsWith('axiom 9.9.9') || fs.readdirSync(path.join(dir, 'inst')).some((f) => f.startsWith('.axiom-update'))
        ? `changed the install: ${v}` : null;
    },
    'a mirror that would be an option, an update running, or one left unfinished: refused': () => {
      const s = skip();
      if (s) return s;
      const exe = install();
      const bad = spawnSync(exe, ['update', '9.9.9', '--format=agent-json'], {
        encoding: 'utf8', env: { ...process.env, AXIOM_UPDATE_URL: '--upload-pack=touch /tmp/x' } });
      let e = code(bad, 2, 'E9001');
      if (e) return `mirror: ${e}`;
      const inst = path.join(dir, 'inst');
      fs.mkdirSync(path.join(inst, '.axiom-update.lock'));
      e = code(update(exe, '9.9.9'), 1, 'E9008');
      if (e) return `lock: ${e}`;
      fs.rmdirSync(path.join(inst, '.axiom-update.lock'));
      fs.mkdirSync(path.join(inst, '.axiom-update-0123/old'), { recursive: true });
      e = code(update(exe, '9.9.9'), 1, 'E9008');
      if (e) return `unfinished: ${e}`;
      const v = spawnSync(exe, ['version'], { encoding: 'utf8' }).stdout;
      return v.startsWith('axiom 9.9.9') ? `updated anyway: ${v}` : null;
    },
    'a release install updates to a release': () => {
      const s = skip();
      if (s) return s;
      const exe = install();
      const r = update(exe, '9.9.9');
      if (r.status) return `${r.status}: ${r.stdout}${r.stderr}`;
      const v = spawnSync(exe, ['version'], { encoding: 'utf8' }).stdout;
      const notes = fs.readFileSync(path.join(dir, 'inst/RELEASE-NOTES.md'), 'utf8');
      const left = fs.readdirSync(path.join(dir, 'inst')).filter((f) => f.startsWith('.axiom-update'));
      return v === 'axiom 9.9.9\n' && notes === 'new\n' && !left.length && fs.existsSync(path.join(root, 'std/mem.ax'))
        ? null : `version ${v}, notes ${notes}, left ${left}`;
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

// Nesting limit (spec §3): expressions, types and blocks nest at most 1000
// levels deep, counting a chain of operators or of postfix operations as one
// level for each link. A program 900 levels deep builds and runs; one 1100
// levels deep is E0110, not a crash.
{
  const fn = (body, before = '') => `${before}export fn main() i32 {\n    let x: i32 = 0;\n${body}\n}\n`;
  const shapes = {
    parens: (n) => fn(`    return ${'('.repeat(n)}x${')'.repeat(n)};`),
    'unary operators': (n) => fn(`    return ${'- '.repeat(n & ~1)}x;`),
    'an operator chain': (n) => fn(`    return ${Array(n).fill('x').join(' + ')};`),
    'nested operator chains': (n) => {
      let e = 'x';
      for (let i = 0; i < n / 10; i++) e = `(${[e, ...Array(9).fill('x')].join(' + ')})`;
      return fn(`    return ${e};`);
    },
    'a postfix chain': (n) => fn(`    let mut a = N.{ n: @ptr_from_int(*N, 8), v: x };\n    a.n = &a;\n    return a${'.n'.repeat(n)}.v;`,
      'struct N {\n    n: *N,\n    v: i32,\n}\n\n'),
    blocks: (n) => fn(`    let mut y: i32 = 0;\n    ${'{'.repeat(n)}y += 1;${'}'.repeat(n)}\n    return y - 1;`),
    types: (n) => fn('    return x;', `fn f(p: ${'*'.repeat(n)}i32) void {\n}\n\n`),
  };
  const cases = {};
  for (const [name, gen] of Object.entries(shapes)) {
    cases[`${name}, 900 levels deep`] = () => {
      const f = path.join(tmp, `nest-ok-${name.replace(/ /g, '_')}.ax`);
      fs.writeFileSync(f, gen(900));
      const out = f.replace(/\.ax$/, native ? '' : '.wasm');
      const c = spawnSync(compiler, ['build', f, '-o', out, ...targetFlags, ...extraFlags], { encoding: 'utf8' });
      if (c.status !== 0) return `build failed (status ${c.status}):\n${c.stdout}${c.stderr}`.slice(0, 2000);
      const r = hostRun(out);
      return r.status !== 0 ? `exit status ${r.status}:\n${r.stderr}` : null;
    };
    cases[`${name}, 1100 levels deep`] = () => {
      const f = path.join(tmp, `nest-deep-${name.replace(/ /g, '_')}.ax`);
      fs.writeFileSync(f, gen(1100));
      const c = spawnSync(compiler, ['check', f, '--format=agent-json'], { encoding: 'utf8' });
      let codes = [];
      try { codes = JSON.parse(c.stdout).diagnostics.map((x) => x.code); } catch { return `bad output (status ${c.status}):\n${c.stdout}${c.stderr}`.slice(0, 2000); }
      return codes[0] === 'E0110' && codes.filter((x) => x === 'E0110').length === 1 ? null : `diagnostics: ${codes.join(', ')}`;
    };
  }
  await section('nesting limit', cases, { timed: false });
}

// Garbage in: a run of characters that are not Axiom is one E0002; the
// parser's placeholder for a missing name leads to no more errors (it is
// not "already declared", does not "shadow"); and at most 1000 diagnostics
// are shown, with the number left out (spec §16.3). The compilers took
// seconds and printed hundreds of megabytes for files like these.
{
  const cases = {};
  const check = (src, name) => {
    const f = path.join(tmp, `garbage-${name}.ax`);
    fs.writeFileSync(f, src);
    const t0 = Date.now();
    const c = spawnSync(compiler, ['check', f, '--format=agent-json'], { encoding: 'utf8', maxBuffer: 1 << 26 });
    try { return { j: JSON.parse(c.stdout), ms: Date.now() - t0 }; } catch { return { err: `bad output (status ${c.status}):\n${c.stdout}${c.stderr}`.slice(0, 2000) }; }
  };
  cases['a run of control characters is one error'] = () => {
    const r = check(`fn f() u64 {\n    return 3;\n}\n${'\x1b'.repeat(20000)}\n`, 'run');
    if (r.err) return r.err;
    const codes = r.j.diagnostics.map((x) => x.code);
    return codes.length === 1 && codes[0] === 'E0002' ? null : `diagnostics: ${codes.slice(0, 20).join(', ')} (${codes.length})`;
  };
  cases['no errors about the placeholder for a missing name'] = () => {
    const r = check(`export fn f(${','.repeat(600)}) void {\n    let = 1;\n    let = 2;\n}\n`, 'names');
    if (r.err) return r.err;
    const bad = r.j.diagnostics.filter((x) => x.message.includes('<error>') || x.code === 'E0801');
    return bad.length === 0 ? null : `${bad.length} cascading: ${bad[0].code} ${bad[0].message}`;
  };
  cases['a long list of missing names is not checked in pairs'] = () => {
    // (20000 parameters the parser could not read: in pairs, 2e8 comparisons)
    const r = check(`fn f(${','.repeat(20000)}) void {\n}\nstruct S {\n${'    : u8,\n'.repeat(5000)}}\n`, 'pairs');
    if (r.err) return r.err;
    return r.ms < 5000 ? null : `took ${r.ms} ms`;
  };
  cases['at most 1000 diagnostics are shown'] = () => {
    const r = check(`fn f() void {\n${'    x;\n'.repeat(3000)}}\n`, 'many');
    if (r.err) return r.err;
    const n = r.j.diagnostics.length;
    return n === 1000 && r.j.omitted === 2000 ? null : `${n} shown, omitted ${r.j.omitted}`;
  };
  await section('garbage in', cases, { timed: false });
}

// The C parser (docs/c-frontend.md; the compiler only): each
// cmd/axiom/internal/tests/c/parse/<name>.c gives the tree in <name>.ast
// (--emit=c-ast), or with `/* expect-error: C2xxx */` directives exactly
// those errors (`/* expect-errors */`: some, any). `/* flags: ... */` adds
// options. Then robustness: truncated copies of the tests, unbalanced
// brackets, deep nesting and token soup end with diagnostics, not a crash.
{
  const dir = path.join(root, 'cmd/axiom/internal/tests/c/parse');
  const cfiles = fs.existsSync(dir) ? fs.readdirSync(dir).filter((f) => f.endsWith('.c')).sort().map((f) => path.join(dir, f)) : [];
  const cdirectives = (src) => ({
    flags: [...src.matchAll(/\/\* flags: (.*?) \*\//g)].flatMap((m) => m[1].split(' ')),
    errors: [...src.matchAll(/\/\* expect-error: (C\d{4}) \*\//g)].map((m) => m[1]),
    anyErrors: /\/\* expect-errors \*\//.test(src),
  });
  const skipBootstrap = () => (bootstrapCompiler() ? { skip: true, reason: 'the compiler only' } : null);
  // The compiler on file f: { status, codes, out } (codes of the errors), or a problem.
  // (the tree on standard output when there are no errors, the diagnostics' JSON when there are)
  const parseC = (f, flags) => {
    const c = spawnSync(compiler, ['build', f, '--emit=c-ast', '--format=agent-json', ...flags], { encoding: 'utf8', timeout: 60000, maxBuffer: 1 << 28 });
    if (c.error) return `${c.error.code || c.error}`;
    if (c.status !== 0 && c.status !== 1) return `status ${c.status} (signal ${c.signal}):\n${c.stdout.slice(0, 2000)}${c.stderr.slice(0, 2000)}`;
    const nl = c.status === 0 ? c.stdout.indexOf('\n') : c.stdout.length;
    let codes = [];
    try { codes = JSON.parse(c.stdout.slice(0, nl)).diagnostics.filter((x) => x.severity === 'error').map((x) => x.code); } catch { return `bad output (status ${c.status}):\n${c.stdout.slice(0, 2000)}${c.stderr}`; }
    return { status: c.status, codes, out: c.stdout.slice(nl + 1) };
  };
  const cases = byFile(cfiles, '', (f) => {
    const s = skipBootstrap();
    if (s) return s;
    const src = fs.readFileSync(f, 'utf8');
    const d = cdirectives(src);
    const r = parseC(f, d.flags);
    if (typeof r === 'string') return r;
    if (d.anyErrors) return r.status === 1 && r.codes.length ? null : 'expected errors, but there were none';
    if (d.errors.length) {
      const want = [...d.errors].sort().join(', '), got = [...r.codes].sort().join(', ');
      return want === got ? null : `expected errors [${want}], got [${got}]:\n${r.out.slice(0, 3000)}`;
    }
    if (r.status !== 0) return `errors [${r.codes.join(', ')}]:\n${r.out.slice(0, 3000)}`;
    const want = fs.readFileSync(f.replace(/\.c$/, '.ast'), 'utf8');
    return r.out === want ? null : `tree mismatch\n--- expected\n${want}\n--- got\n${r.out}`;
  });
  const write = (name, text) => {
    const f = path.join(tmp, `cparse-${name}.c`);
    fs.writeFileSync(f, text);
    return f;
  };
  // ends with errors (any, or exactly one `code` when given), in a few seconds at most
  const fails = (name, text, code, flags = []) => () => {
    const s = skipBootstrap();
    if (s) return s;
    const t0 = Date.now();
    const r = parseC(write(name, text), flags);
    if (typeof r === 'string') return r;
    if (Date.now() - t0 > 10000) return `took ${Date.now() - t0} ms`;
    if (r.status !== 1 || !r.codes.length) return 'no errors';
    if (code && (r.codes.length !== 1 || r.codes[0] !== code)) return `expected one ${code}, got [${r.codes.slice(0, 20).join(', ')}]`;
    return null;
  };
  const passes = (name, text, flags = []) => () => {
    const s = skipBootstrap();
    if (s) return s;
    const r = parseC(write(name, text), flags);
    if (typeof r === 'string') return r;
    return r.status === 0 ? null : `errors [${r.codes.slice(0, 20).join(', ')}]`;
  };
  const fn = (body) => `int f(int x) {\n${body}\n}\n`;
  const deep = {
    parentheses: (n) => fn(`return ${'('.repeat(n)}x${')'.repeat(n)};`),
    'unary operators': (n) => fn(`return ${'- '.repeat(n)}x;`),
    casts: (n) => fn(`return ${'(int)'.repeat(n)}x;`),
    'an operator chain': (n) => fn(`return ${Array(n).fill('x').join(' + ')};`),
    assignments: (n) => fn(`${Array(n).fill('x').join(' = ')} = 1; return x;`),
    conditionals: (n) => fn(`return ${'x ? x : '.repeat(n)}x;`),
    'a postfix chain': (n) => fn(`return x${'[0]'.repeat(n)};`),
    blocks: (n) => fn(`${'{'.repeat(n)}x++;${'}'.repeat(n)} return x;`),
    'else-if chains': (n) => fn(`${'if (x) x++; else '.repeat(n)}x--; return x;`),
    initializers: (n) => `int a = ${'{'.repeat(n)}1${'}'.repeat(n)};\n`,
    declarators: (n) => `int ${'('.repeat(n)}a${')'.repeat(n)};\n`,
    'function declarators': (n) => `int a${'(int'.repeat(n)}${')'.repeat(n)};\n`,
    structures: (n) => `${'struct { '.repeat(n)}int x;${' } a;'.repeat(n)}\n`,
    'typeof operands': (n) => `int x;\n${'typeof('.repeat(n)}x${')'.repeat(n)} y;\n`,
    'compound literals': (n) => fn(`return ${'(int[]){ '.repeat(n)}1${' }[0]'.repeat(n)};`),
  };
  for (const [name, gen] of Object.entries(deep)) {
    const flags = name === 'typeof operands' ? ['--c-std=c23'] : [];
    cases[`${name}, 400 levels deep`] = passes(`ok-${name.replace(/ /g, '_')}`, gen(400), flags);
    cases[`${name}, 2000 levels deep, is C2002`] = fails(`deep-${name.replace(/ /g, '_')}`, gen(2000), 'C2002', flags);
  }
  cases['unbalanced brackets'] = fails('unbalanced', `${'int f(void) { ( [ { '.repeat(300)}\nint g(void) { return 0; } ) ] } }\n`);
  cases['only closing brackets'] = fails('closing', ') ] } '.repeat(2000));
  cases['a file of opening braces'] = fails('braces', '{'.repeat(100000));
  // token soup: random tokens from a fixed seed
  {
    const words = ['int', 'char', 'struct', 'union', 'enum', 'typedef', 'static', 'const', 'x', 'y', 'T', '(', ')', '[', ']',
      '{', '}', ';', ',', '*', '=', '+', '?', ':', '.', '->', '...', '1', '"s"', "'c'", 'if', 'else', 'for', 'while', 'do',
      'return', 'case', 'default', 'goto', 'sizeof', '_Generic', '_Atomic', '[[', ']]', '::', 'typeof', 'alignas', '_BitInt'];
    for (let seed = 1; seed <= 6; seed++) {
      let st = seed * 2654435761 >>> 0;
      const rnd = () => ((st = (st * 1103515245 + 12345) >>> 0) >>> 8) % words.length;
      const toks = ['typedef int T;'];
      for (let i = 0; i < 20000; i++) toks.push(words[rnd()]);
      cases[`token soup ${seed}`] = fails(`soup${seed}`, toks.join(' ') + '\n', null, seed % 2 ? ['--c-std=c89'] : ['--c-std=c23']);
    }
  }
  // truncated copies of the tests: diagnostics or a tree, nothing else
  cases['the tests, truncated'] = () => {
    const s = skipBootstrap();
    if (s) return s;
    for (const f of cfiles) {
      const src = fs.readFileSync(f, 'utf8');
      const d = cdirectives(src);
      for (const frac of [0.3, 0.6, 0.9]) {
        const cut = Math.floor(src.length * frac);
        const r = parseC(write('cut', src.slice(0, cut)), d.flags);
        if (typeof r === 'string') return `${path.basename(f)} cut at ${cut}: ${r}`;
      }
    }
    return null;
  };
  await section('C parser', cases, { parallel: true, timed: false });
}

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

// Debug information (-g; the compiler only): cmd/axiom/internal/tests/dwarf.mjs
// reads the DWARF of the programs of cmd/axiom/internal/tests/debug, built for
// each target at -O0 and -O2, and checks its structure (units, a subprogram
// and a covering line sequence per function, call frame information per
// function agreeing with the frame tables' calls); that marked lines are in
// the line tables (and C's #line followed); that two builds are the same; and
// that -g changes no code: an ELF executable's loaded bytes, a Mach-O
// executable (the DWARF is in its .dSYM), a WebAssembly module but its
// .debug_* sections, an object's .text are the same as without it.
if (!skipPlatform) {
  const ddir = path.join(root, 'cmd/axiom/internal/tests/debug');
  const markLine = (src, mark) => fs.readFileSync(src, 'utf8').split('\n').findIndex((l) => l.includes(`line: ${mark}`)) + 1;
  // the bytes a loader maps of an ELF executable (up to its last PT_LOAD's file end)
  const loaded = (d) => {
    const phoff = Number(d.readBigUInt64LE(32)), phnum = d.readUInt16LE(56);
    let end = 0;
    for (let k = 0; k < phnum; k++) {
      const h = phoff + 56 * k;
      if (d.readUInt32LE(h) === 1) end = Math.max(end, Number(d.readBigUInt64LE(h + 8) + d.readBigUInt64LE(h + 32)));
    }
    const b = Buffer.from(d.subarray(0, end));
    b.fill(0, 40, 48);  // (e_shoff, e_shnum, e_shstrndx: the section headers come after it)
    b.fill(0, 60, 64);
    return b;
  };
  // a WebAssembly module without its .debug_* custom sections
  const wasmCode = (d) => {
    const out = [d.subarray(0, 8)];
    let p = 8;
    const uleb = () => { let r = 0, s = 0, b; do { b = d[p++]; r += (b & 0x7f) * 2 ** s; s += 7; } while (b & 0x80); return r; };
    while (p < d.length) {
      const at = p, id = d[p++], size = uleb(), end = p + size;
      let keep = true;
      if (id === 0) { const n = uleb(); keep = !d.subarray(p, p + n).toString().startsWith('.debug_'); }
      if (keep) out.push(d.subarray(at, end));
      p = end;
    }
    return Buffer.concat(out);
  };
  // an ELF object's .text relocations, normalised: offset, type, the
  // symbol's name (a section symbol's: its section's), addend (the symbol
  // tables' order and size differ with the debug sections)
  const objRelocs = (d) => {
    if (isCoff(d)) return coffSecs(d).filter((x) => x.name === '.text').flatMap((x) => x.rels).map((x) => x.replace(/\$L\d+_/, '$L_')).join('\n');
    const shoff = Number(d.readBigUInt64LE(40)), shnum = d.readUInt16LE(60), shstr = d.readUInt16LE(62);
    const sh = (k) => shoff + 64 * k;
    const so = Number(d.readBigUInt64LE(sh(shstr) + 24));
    const secName = (k) => d.toString('latin1', so + d.readUInt32LE(sh(k)), d.indexOf(0, so + d.readUInt32LE(sh(k))));
    let text = -1;
    for (let k = 0; k < shnum; k++) if (secName(k) === '.text') text = k;
    const out = [];
    for (let k = 0; k < shnum; k++) {
      if (d.readUInt32LE(sh(k) + 4) !== 4 || d.readUInt32LE(sh(k) + 44) !== text) continue;  // (SHT_RELA of .text)
      const symtab = d.readUInt32LE(sh(k) + 40), st = Number(d.readBigUInt64LE(sh(symtab) + 24));
      const strtab = Number(d.readBigUInt64LE(sh(d.readUInt32LE(sh(symtab) + 40)) + 24));
      const at = Number(d.readBigUInt64LE(sh(k) + 24)), n = Number(d.readBigUInt64LE(sh(k) + 32)) / 24;
      for (let j = 0; j < n; j++) {
        const r = at + 24 * j, info = d.readBigUInt64LE(r + 8), si = Number(info >> 32n), y = st + 24 * si;
        const nm = d.readUInt32LE(y), name = nm ? d.toString('latin1', strtab + nm, d.indexOf(0, strtab + nm)) : `[${secName(d.readUInt16LE(y + 6))}]`;
        out.push(`${d.readBigUInt64LE(r)} ${info & 0xffffffffn} ${name} ${d.readBigInt64LE(r + 16)}`);
      }
    }
    return out.join('\n');
  };
  // a COFF object's sections: name, raw data, relocations (offset, type, symbol name)
  const isCoff = (d) => [0x8664, 0xaa64].includes(d.readUInt16LE(0)) && d.readUInt16LE(16) === 0;
  const coffSecs = (d) => {
    const nsec = d.readUInt16LE(2), symoff = d.readUInt32LE(8), nsym = d.readUInt32LE(12), strs = symoff + 18 * nsym;
    const nameAt = (o) => d.readUInt32LE(o) === 0 ? d.toString('latin1', strs + d.readUInt32LE(o + 4), d.indexOf(0, strs + d.readUInt32LE(o + 4)))
      : d.toString('latin1', o, o + 8).replace(/\0.*$/, '');
    const out = [];
    for (let k = 0; k < nsec; k++) {
      const h = 20 + 40 * k;
      let name = d.toString('latin1', h, h + 8).replace(/\0.*$/, '');
      if (name.startsWith('/')) name = d.toString('latin1', strs + Number(name.slice(1)), d.indexOf(0, strs + Number(name.slice(1))));
      const raw = d.subarray(d.readUInt32LE(h + 20), d.readUInt32LE(h + 20) + d.readUInt32LE(h + 16));
      const rels = [];
      for (let j = 0; j < d.readUInt16LE(h + 32); j++) {
        const r = d.readUInt32LE(h + 24) + 10 * j;
        rels.push(`${d.readUInt32LE(r)} ${d.readUInt16LE(r + 8)} ${nameAt(symoff + 18 * d.readUInt32LE(r + 4))}`);
      }
      out.push({ name, raw, rels });
    }
    return out;
  };
  // an ELF (or COFF) object's .text
  const objText = (d) => {
    if (isCoff(d)) return Buffer.concat(coffSecs(d).filter((x) => x.name === '.text').map((x) => x.raw));
    const shoff = Number(d.readBigUInt64LE(40)), shnum = d.readUInt16LE(60), shstr = d.readUInt16LE(62);
    const so = Number(d.readBigUInt64LE(shoff + 64 * shstr + 24));
    for (let k = 0; k < shnum; k++) {
      const h = shoff + 64 * k;
      if (d.subarray(so + d.readUInt32LE(h), so + d.readUInt32LE(h) + 6).toString() === '.text\0') {
        const o = Number(d.readBigUInt64LE(h + 24));
        return d.subarray(o, o + Number(d.readBigUInt64LE(h + 32)));
      }
    }
    return Buffer.alloc(0);
  };
  // the size of a Mach-O executable's __TEXT,__text section
  const machoText = (d) => {
    const ncmds = d.readUInt32LE(16);
    for (let k = 0, at = 32; k < ncmds; k++) {
      const cmd = d.readUInt32LE(at), size = d.readUInt32LE(at + 4);
      if (cmd === 0x19) {
        const nsect = d.readUInt32LE(at + 64);
        for (let j = 0; j < nsect; j++) {
          const sh = at + 72 + 80 * j;
          if (d.toString('latin1', sh, sh + 16).replace(/\0+$/, '') === '__text') return Number(d.readBigUInt64LE(sh + 40));
        }
      }
      at += size;
    }
    return -1;
  };
  const dcases = {};
  const targets = ['x86_64', 'aarch64', 'aarch64-macos', 'x86_64-macos', 'wasm32', 'wasm64'];
  for (const [lang, file] of [['c', 'frames.c'], ['ax', 'frames.ax'], ['c', 'vars.c'], ['ax', 'vars.ax'], ['ax', 'shared.ax']]) {
    const src = path.join(ddir, file);
    for (const t of targets) {
      for (const lvl of ['-O0', '-O2']) {
        for (const obj of t.endsWith('64') && !t.startsWith('wasm') ? [false, true] : [false]) {
          dcases[`${file} ${t} ${lvl}${obj ? ' object' : ''}`] = async () => {
            const base = path.join(tmp, `debug-${file}-${t}${lvl}${obj ? '-o' : ''}`);
            // (the same name: a Mach-O executable's signature names it)
            const outs = ['g1', 'g2', 'n'].map((k) => { fs.mkdirSync(`${base}-${k}`, { recursive: true }); return path.join(`${base}-${k}`, 'prog'); });
            const env = { ...process.env, AXIOM_CACHE: path.join(tmp, `debug-cache-${t}`) };
            for (const [k, o] of outs.entries()) {
              const r = await spawnAsync(compiler, ['build', src, `--target=${t}`, lvl, ...(k < 2 ? ['-g'] : []),
                ...(obj ? ['--emit=obj'] : []), '-o', o], { encoding: 'utf8', cwd: root, env, timeout: compileTimeout });
              if (r.status !== 0) return `build failed:\n${r.stdout}${r.stderr}`;
            }
            const mac = t.endsWith('macos');
            const dfile = (o) => (mac ? path.join(`${o}.dSYM/Contents/Resources/DWARF`, path.basename(o)) : o);
            const [a, b, n] = outs.map((o, k) => fs.readFileSync(k < 2 ? dfile(o) : o));
            if (!a.equals(b)) return 'two builds differ';
            if (mac && fs.existsSync(`${outs[2]}.dSYM`)) return 'a .dSYM without -g';
            const [ea, en] = [fs.readFileSync(outs[0]), fs.readFileSync(outs[2])];
            // (the instructions are the same: an object's bytes and its
            // relocations (normalised); an executable's code, where the frame
            // table's line rows (ftab.ax) move the data after them and so the
            // addresses its instructions hold, its size)
            const same = mac ? machoText(ea) === machoText(en) : t.startsWith('wasm') ? wasmCode(ea).equals(wasmCode(en))
              : obj ? objText(ea).equals(objText(en)) && objRelocs(ea) === objRelocs(en) && objRelocs(ea).length > 0
              : objText(ea).length === objText(en).length && objText(ea).length > 0;
            void loaded;
            if (!same) return '-g changed the code';
            if (mac && checkMachO(ea).errs.length) return `the executable: ${checkMachO(ea).errs.join('\n')}`;
            if (!t.startsWith('wasm') && !mac && !obj && n.length === a.length) return 'no debug information';
            const img = loadImage(a);
            const r = checkDebug(img, { cfi: !t.startsWith('wasm') });
            if (r.errs.length) return r.errs.join('\n');
            // the line tables' MD5 checksums: each a file's on disk (none in a
            // unit with a name of C's #line, frames.c's)
            let summed = 0;
            for (const sq of r.dw.seqs) {
              sq.files.forEach((f, k) => {
                if (sq.sums[k] && fs.existsSync(f)) {
                  summed++;
                  if (!crypto.createHash('md5').update(fs.readFileSync(f)).digest().equals(sq.sums[k])) r.errs.push(`${f}: not its MD5`);
                }
              });
            }
            if (r.errs.length) return r.errs.slice(0, 10).join('\n');
            if (!summed && file !== 'frames.c') return 'no MD5 checksums';
            if (obj && !r.nrets) return 'no frame table entries checked';
            if (file === 'vars.c') {
              // (WebAssembly: the variables at -O0, in their frame slots from the
              // frame's local; optimized code's are not described there, and a
              // thread-local has no one location)
              const wasm = t.startsWith('wasm');
              if (wasm && lvl !== '-O0') return r.nvars ? null : 'no variables';
              const errs = checkVars(r.dw, { O0: lvl === '-O0', tls: !mac && !wasm, src, markLine, machine: t.startsWith('x86') ? 'x64' : 'a64' });
              return errs.length ? errs.join('\n') : null;
            }
            if (file === 'shared.ax') return null;  // (checkDebug: every function has its subprogram)
            if (file === 'vars.ax') {
              if (lvl !== '-O0') return null;
              const errs = checkAxVars(r.dw);
              return errs.length ? errs.join('\n') : null;
            }
            const lines = lang === 'c' ? ['add-sum', 'many-loop', 'vla-decl', 'tail-call'] : ['many-loop', 'tail-call'];
            for (const m of lines) {
              const at = addrsOfLine(r.dw, `frames.${lang}`, markLine(src, m));
              if (!at.length) return `no code for the line marked ${m}`;
            }
            if (lang === 'c' && lvl === '-O0') {
              const at = addrsOfLine(r.dw, 'generated.y', 501);
              if (!at.length) return 'no code for generated.y:501 (#line)';
              const sp = subprogramAt(r.dw, at[0]);
              if (!sp || sp.attrs.get(0x03) !== 'presumed') return 'generated.y:501 is not in presumed';
            }
            return null;
          };
        }
      }
    }
  }
  // Windows (x64 and ARM64): the program database (pdb.mjs: its structure,
  // every type index resolvable, every scope closed, every location's range
  // in its procedure), two builds the same, -g changing no code, and
  // vars.c's variables there (at -O0 each with a location).
  for (const file of ['frames.c', 'frames.ax', 'vars.c', 'vars.ax', 'shared.ax']) {
    const src = path.join(ddir, file);
    for (const t of ['x86_64-windows', 'aarch64-windows']) {
      for (const lvl of ['-O0', '-O2']) {
        dcases[`${file} ${t} ${lvl}`] = async () => {
          const dir = path.join(tmp, `debug-${file}-${t}${lvl}`);
          fs.mkdirSync(dir, { recursive: true });
          const exe = path.join(dir, 'prog.exe'), pdbf = path.join(dir, 'prog.pdb');
          const env = { ...process.env, AXIOM_CACHE: path.join(tmp, `debug-cache-${t}`) };
          const got = [];
          for (const g of [true, true, false]) {
            const r = await spawnAsync(compiler, ['build', src, `--target=${t}`, lvl, ...(g ? ['-g'] : []), '-o', exe],
              { encoding: 'utf8', cwd: root, env, timeout: compileTimeout });
            if (r.status !== 0) return `build failed:\n${r.stdout}${r.stderr}`;
            if (g && got.length === 0) {
              const pe = checkPEFile(exe, { arm64: t.startsWith('aarch64') });
              if (pe.errs.length) return pe.errs.slice(0, 20).join('\n');
            }
            got.push([fs.readFileSync(exe), fs.existsSync(pdbf) ? fs.readFileSync(pdbf) : Buffer.alloc(0)]);
          }
          if (!got[0][0].equals(got[1][0]) || !got[0][1].equals(got[1][1])) return 'two builds differ';
          const text = (d) => {
            const pe = d.readUInt32LE(0x3c), h = pe + 24 + d.readUInt16LE(pe + 20);
            return d.toString('latin1', h, h + 5) === '.text' ? d.subarray(d.readUInt32LE(h + 20), d.readUInt32LE(h + 20) + d.readUInt32LE(h + 8)) : Buffer.alloc(0);
          };
          if (text(got[0][0]).length !== text(got[2][0]).length || !text(got[0][0]).length) return '-g changed the code';
          // (the objects' instructions and relocations, normalised: the same)
          const objs = [];
          for (const g of [true, false]) {
            const o = path.join(dir, `prog${g ? '-g' : ''}.o`);
            const r = await spawnAsync(compiler, ['build', src, `--target=${t}`, lvl, ...(g ? ['-g'] : []), '--emit=obj', '-o', o],
              { encoding: 'utf8', cwd: root, env, timeout: compileTimeout });
            if (r.status !== 0) return `build failed:\n${r.stdout}${r.stderr}`;
            objs.push(fs.readFileSync(o));
          }
          if (!objText(objs[0]).equals(objText(objs[1])) || objRelocs(objs[0]) !== objRelocs(objs[1]) || !objRelocs(objs[0]).length) return '-g changed the object\'s code';
          const r = checkPDB(got[0][1]);
          if (r.errs.length) return r.errs.slice(0, 20).join('\n');
          if (file !== 'vars.c') return null;
          // the variables of vars.c's functions (a block's own x too); at -O0
          // each with a location (no S_LOCAL without one: the VLA's v, held
          // through its address, has none)
          const want = { shadow: ['n', 'x', 'px', 'i', 'pr', 'w'], once: ['a', 'b', 's', 't'], vla: ['n', 'i'] };
          for (const [fn, names] of Object.entries(want)) {
            const p = r.procs.find((x) => x.name === fn);
            if (!p) return `no procedure ${fn}`;
            for (const n of names) {
              const l = p.locals.find((x) => x.name === n);
              if (!l) return `${fn}: no variable ${n}`;
              if (lvl === '-O0' && (l.flags & 0x100)) return `${fn}: ${n} has no location at -O0`;
            }
            if (fn === 'shadow' && p.locals.filter((x) => x.name === 'x').length < 2) return 'shadow: the inner x is missing';
          }
          if (!r.types.some((x) => x.readUInt16LE(2) === 0x1505 && x.includes(Buffer.from('pair\0')))) return 'no struct pair';
          return null;
        };
      }
    }
  }
  // call frame information of assembly (debug/asm_cfi.c): a file-scope asm
  // procedure's .cfi_* directives, and an inline template's saved register
  for (const t of ['x86_64', 'aarch64', 'x86_64-macos', 'aarch64-macos']) {
    for (const lvl of ['-O0', '-O2']) {
      dcases[`asm_cfi.c ${t} ${lvl}`] = async () => {
        const src = path.join(ddir, 'asm_cfi.c');
        const o = path.join(tmp, `debug-asm-cfi-${t}${lvl}`, 'prog');
        fs.mkdirSync(path.dirname(o), { recursive: true });
        const env = { ...process.env, AXIOM_CACHE: path.join(tmp, `debug-cache-${t}`) };
        const r = await spawnAsync(compiler, ['build', src, `--target=${t}`, lvl, '-g', '--c-std=gnu17', '-o', o],
          { encoding: 'utf8', cwd: root, env, timeout: compileTimeout });
        if (r.status !== 0) return `build failed:\n${r.stdout}${r.stderr}`;
        const mac = t.endsWith('macos'), x64 = t.startsWith('x86');
        const img = loadImage(fs.readFileSync(mac ? path.join(`${o}.dSYM/Contents/Resources/DWARF`, 'prog') : o));
        const dw = readDwarf(img);
        const fn = (n) => (img.funcs || []).find((f) => f.name === n);
        const fdeOf = (f) => dw.fdes.find((d) => d.low === f.addr);
        const show = (st) => `CFA ${st.cfa} ${[...st.regs].map(([k, v]) => `${k}:${v}`).join(' ')}`;
        const isExpr = (x, r) => x.regs.has(r) && x.regs.get(r)[0] === 'expr';
        // (Mach-O's symbols have no sizes: asmf's FDE is the one of its size, inl's the one with expressions)
        const asmf = mac ? dw.fdes.filter((d) => d.high - d.low === (x64 ? 17 : 28)).map((d) => ({ addr: d.low, size: d.high - d.low }))[0] : fn('asmf');
        const inl = mac ? dw.fdes.filter((d) => d.rows.some((x) => isExpr(x, x64 ? 3 : 19))).map((d) => ({ addr: d.low }))[0] : fn('inl');
        if (!asmf || !inl) return 'no symbol asmf or inl';
        const fa = fdeOf(asmf);
        if (!fa || fa.high !== asmf.addr + asmf.size) return 'no FDE (or of another size) for asmf';
        // (offset, CFA register and offset, a register and its rule) at points of asmf
        const want = x64
          ? [[0, 7, 8, 3, 'u'], [1, 7, 16, 3, 'off,-16'], [3, 7, 24, 12, 'off,-24'], [7, 7, 32, 3, 'off,-16'],
             [13, 7, 24, 12, 'off,-24'], [15, 7, 16, 12, 'same'], [16, 7, 8, 3, 'same']]
          : [[0, 31, 0, 29, 'u'], [4, 31, 32, 30, 'off,-24'], [8, 29, 32, 29, 'off,-32'], [12, 29, 32, 19, 'off,-16'],
             [20, 29, 32, 19, 'same'], [24, 31, 0, 30, 'same']];
        for (const [off, cr, co, reg, rule] of want) {
          const st = frameAt(fa, asmf.addr + off);
          const got = st.regs.has(reg) ? String(st.regs.get(reg)) : 'u';
          if (st.cfa[0] !== cr || st.cfa[1] !== co || got !== rule) return `asmf+${off}: ${show(st)}`;
        }
        // inl: the saved register's rule is an expression (from the stack
        // pointer) while the template has it on the stack, and its own
        // value again at the end; the CFA is the frame pointer's throughout
        const fi = fdeOf(inl);
        if (!fi) return 'no FDE for inl';
        const reg = x64 ? 3 : 19, fp = x64 ? 6 : 29;
        const exprs = fi.rows.filter((x) => x.regs.has(reg) && x.regs.get(reg)[0] === 'expr');
        if (exprs.length < (x64 ? 2 : 1)) return `inl: no rows with the saved register: ${fi.rows.map(show).join('; ')}`;
        if (exprs.some((x) => x.cfa[0] !== fp)) return 'inl: the CFA is not the frame pointer\'s while the template runs';
        // (x86-64: at rsp + 0 after the push, at rsp + 8 after the second)
        if (x64 && (exprs[0].regs.get(reg)[1] !== '7700' || !exprs.some((x) => x.regs.get(reg)[1] === '7708')))
          return `inl: the saved register's rules: ${exprs.map(show).join('; ')}`;
        const after = fi.rows[fi.rows.indexOf(exprs[exprs.length - 1]) + 1];
        if (!after || (after.regs.has(reg) && after.regs.get(reg)[0] === 'expr')) return 'inl: the register is not given back';
        return null;
      };
    }
  }
  // line information of assembly sources: each statement's line (.S and
  // .s), and what .file and .loc say where they do
  for (const t of ['x86_64', 'aarch64', 'x86_64-macos', 'aarch64-macos']) {
    dcases[`assembly sources' lines ${t}`] = async () => {
      const dir = path.join(root, 'cmd/axiom/internal/tests/c/mix/asm_units');
      const loc = path.join(tmp, `debug-asm-loc-${t}.s`);
      fs.writeFileSync(loc, '    .file 1 "virtual.c"\n    .text\n    .globl asm_loc\nasm_loc:\n    .loc 1 42\n' +
        (t.startsWith('x86') ? '    nop\n    ret\n' : '    nop\n    ret\n'));
      const o = path.join(tmp, `debug-asm-lines-${t}`, 'prog.o');
      fs.mkdirSync(path.dirname(o), { recursive: true });
      const env = { ...process.env, AXIOM_CACHE: path.join(tmp, `debug-cache-${t}`) };
      const r = await spawnAsync(compiler, ['build', path.join(dir, 'calls.c'), path.join(dir, 'routines.S'), path.join(dir, 'table.s'), loc,
        `--target=${t}`, '-g', '--emit=obj', '-o', o], { encoding: 'utf8', cwd: root, env, timeout: compileTimeout });
      if (r.status !== 0) return `build failed:\n${r.stdout}${r.stderr}`;
      const dw = readDwarf(loadImage(fs.readFileSync(o)));
      const lines = fs.readFileSync(path.join(dir, 'routines.S'), 'utf8').split('\n');
      const first = lines.findIndex((l) => l.includes(t.startsWith('x86') ? 'xorl %eax, %eax' : 'mov x2, #0')) + 1;
      const loop = lines.findIndex((l) => l.includes(t.startsWith('x86') ? 'addq (A0), %rax' : 'ldr x3, [x0], #8')) + 1;
      for (const n of [first, loop]) if (!addrsOfLine(dw, 'routines.S', n).length) return `no code for routines.S:${n}`;
      if (!addrsOfLine(dw, 'virtual.c', 42).length) return 'no code for virtual.c:42 (.loc)';
      return null;
    };
  }
  await section('debug information', dcases, {
    parallel: true,
    supported: () => (bootstrapCompiler() ? 'the compiler\'s own code generators only' : null),
  });
}

// macOS (spec §14.1; the compiler only, not the bootstrap one). cmd/axiom/internal/tests/macho.mjs checks
// A C program of two translation units over the platform's C library (for the
// macos and windows sections): printf, an atexit function (returning from
// main is calling exit), the arguments, static functions of the same name.
const platformC = (n) => {
  const a = path.join(tmp, `${n}-c-a.c`), b = path.join(tmp, `${n}-c-b.c`);
  fs.writeFileSync(a, 'int printf(const char *, ...);\nint atexit(void (*)(void));\nstatic int f(void) { return 1; }\n' +
    'int g(void);\nstatic void bye(void) { printf("bye\\n"); }\n' +
    'int main(int argc, char **argv) {\n    atexit(bye);\n    printf("%s %d %.2f\\n", argv[1], f() + g(), 2.5);\n    return 3;\n}\n');
  fs.writeFileSync(b, 'static int f(void) { return 40; }\nint g(void) { return f() + 1; }\n');
  return [a, b];
};

// the executables built for --target=aarch64-macos the way dyld and the
// kernel would; the compiler itself is among them. On a Mac they also run
// (and the whole suite runs natively with AXIOM_NATIVE_TARGET=aarch64-macos);
// elsewhere `axiom test` for the target says it needs a Mac.
if (!skipPlatform) {
  const onMac = process.platform === 'darwin' && os.arch() === 'arm64';
  const out = (n) => path.join(tmp, 'macos-' + n);
  const build = (src, n, extra = [], t = 'aarch64-macos') => spawnSync(compiler, ['build', src, `--target=${t}`, '-o', out(n), ...extra],
    { encoding: 'utf8', cwd: root, env: { ...process.env, AXIOM_CACHE: path.join(tmp, `macos-cache-${t}`) } });
  const check = (src, n, extra, t = 'aarch64-macos') => {
    const c = build(src, n, extra, t);
    if (c.status !== 0) return `build failed:\n${c.stdout}${c.stderr}`;
    const r = checkMachO(fs.readFileSync(out(n)), { x86_64: t === 'x86_64-macos' });
    if (r.errs.length) return r.errs.join('\n');
    return (fs.statSync(out(n)).mode & 0o111) ? null : 'not executable';
  };
  // x86_64-macos (an Intel Mac, or Apple silicon's translation of it): built
  // and checked; nothing here has run it on a Mac yet
  const onIntelMac = process.platform === 'darwin' && os.arch() === 'x64';
  const x86 = (src, n, extra = []) => check(src, n, extra, 'x86_64-macos');
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
    'a C program: two translation units, the C library': () => {
      const [a, b] = platformC('macos');
      const p = check(a, 'c', [b]);
      if (p || !onMac) return p;
      const r = spawnSync(out('c'), ['x'], { encoding: 'utf8' });
      return r.status === 3 && r.stdout === 'x 42 2.50\nbye\n' ? null : `status ${r.status}: ${r.stdout}${r.stderr}`;
    },
    'axiom test': () => {
      const r = spawnSync(compiler, ['test', path.join(root, 'std/str.ax'), '--target=aarch64-macos'], { encoding: 'utf8', cwd: root });
      if (!onMac) return r.status !== 0 && /E9001/.test(r.stderr) ? null : `axiom test: status ${r.status}: ${r.stderr}`;
      return r.status === 0 && /passed, 0 failed/.test(r.stdout) ? null : `axiom test: status ${r.status}: ${r.stdout}${r.stderr}`;
    },
    'x86_64-macos: thread-locals and output': () => {
      const src = path.join(tmp, 'macos-hello.ax');
      fs.writeFileSync(src, 'import "std/io" as io;\n\nthreadlocal let mut tl: u64 = 41;\n\nexport fn main() i32 {\n' +
        '    tl += 1;\n    io.stdout().println("hello") catch return 1;\n    return @int_cast(i32, tl) - 42;\n}\n');
      const p = x86(src, 'x-hello', ['-O2']);
      if (p || !onIntelMac) return p;
      const r = spawnSync(out('x-hello'), [], { encoding: 'utf8' });
      return r.status === 0 && r.stdout === 'hello\n' ? null : `status ${r.status}: ${r.stdout}${r.stderr}`;
    },
    'x86_64-macos: examples, at -O0 and -O2, with -g': () => {
      for (const e of ['wordcount', 'http_server', 'texmath', 'pi']) {
        for (const o of ['-O0', '-O2']) {
          const p = x86(path.join(root, `examples/${e}/${e}.ax`), `x-${e}${o}`, [o, '-g']);
          if (p) return `${e} ${o}: ${p}`;
          if (!fs.existsSync(`${out(`x-${e}${o}`)}.dSYM`)) return `${e} ${o}: no .dSYM`;
        }
      }
      return null;
    },
    'x86_64-macos: the compiler': () => x86(path.join(root, 'cmd/axiom/src/main.ax'), 'x-axiom', ['-O2']),
    'x86_64-macos: frameworks checked, examples on Metal built (structs on the stack: std/os/objc)': () => {
      for (const f of fs.readdirSync(path.join(root, 'cmd/axiom/internal/tests/macos')).filter((n) => n.endsWith('.ax')).sort()) {
        const c = spawnSync(compiler, ['check', `cmd/axiom/internal/tests/macos/${f}`, '--target=x86_64-macos'], { encoding: 'utf8', cwd: root });
        if (c.status !== 0) return `${f}: check failed:\n${c.stdout}${c.stderr}`;
      }
      for (const e of ['metal', 'game']) {
        const p = x86(path.join(root, `examples/${e}/${e}.ax`), `x-${e}`, ['-O2']);
        if (p) return `${e}: ${p}`;
      }
      return null;
    },
    'x86_64-macos: a C program: two translation units, the C library': () => {
      const [a, b] = platformC('macos-x');
      const p = x86(a, 'x-c', [b]);
      if (p || !onIntelMac) return p;
      const r = spawnSync(out('x-c'), ['x'], { encoding: 'utf8' });
      return r.status === 3 && r.stdout === 'x 42 2.50\nbye\n' ? null : `status ${r.status}: ${r.stdout}${r.stderr}`;
    },
    'x86_64-macos: the 64-bit inode functions bound by their variants': () => {
      const c = build(path.join(root, 'examples/wordcount/wordcount.ax'), 'x-inode', [], 'x86_64-macos');
      if (c.status !== 0) return `build failed:\n${c.stdout}${c.stderr}`;
      const r = checkMachO(fs.readFileSync(out('x-inode')));
      for (const f of ['_stat$INODE64', '_fstat$INODE64', '_readdir$INODE64', '_opendir$INODE64']) if (!r.imports.includes(f)) return `no import ${f}`;
      return r.imports.includes('_stat') ? 'an import of _stat (32-bit inodes)' : null;
    },
    'x86_64-macos: axiom test --no-run': () => {
      const t = out('x-tests');
      const r = spawnSync(compiler, ['test', path.join(root, 'std/str.ax'), '--target=x86_64-macos', `--no-run=${t}`], { encoding: 'utf8', cwd: root });
      if (r.status !== 0) return `axiom test --no-run: status ${r.status}: ${r.stdout}${r.stderr}`;
      const e = checkMachO(fs.readFileSync(t), { x86_64: true }).errs;
      if (e.length) return e.join('\n');
      const q = spawnSync(compiler, ['test', path.join(root, 'std/str.ax'), '--target=x86_64-macos'], { encoding: 'utf8', cwd: root });
      return onMac || (q.status !== 0 && /E9001/.test(q.stderr)) ? null : `axiom test: status ${q.status}: ${q.stderr}`;
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
  const build = (src, n, extra = [], t = 'x86_64-windows') => spawnSync(compiler, ['build', src, `--target=${t}`, '-o', out(n), ...extra],
    { encoding: 'utf8', cwd: root, env: t === 'x86_64-windows' ? env : { ...env, AXIOM_CACHE: path.join(tmp, `windows-cache-${t}`) } });
  const wine = ['wine64', 'wine', '/usr/lib/wine/wine64'].find((w) => spawnSync(w, ['--version'], { encoding: 'utf8' }).status === 0);
  // The headers and tables (pe.mjs; for ARM64 the code and its unwind
  // information too, with the PDB when there is one).
  // No program imports anything of the system's C library (docs/c.md).
  const checkPE = (f, arm64 = false) => {
    const r = checkPEFile(f, { arm64 });
    const crt = r.imports ? r.imports.filter((x) => /^(msvcrt|ucrtbase)\.dll:/i.test(x)) : [];
    if (crt.length) r.errs.push(`imports from the system's C library: ${crt.join(' ')}`);
    return r.errs.length ? r.errs.join('\n') : null;
  };
  // aarch64-windows: built and checked; Wine runs x86-64 programs only, and
  // nothing here has run one on Windows on ARM64 yet
  const onArm64Windows = process.platform === 'win32' && os.arch() === 'arm64';
  const arm = (src, n, extra = []) => {
    const c = build(src, n, extra, 'aarch64-windows');
    if (c.status !== 0) return `build failed:\n${c.stdout}${c.stderr}`;
    return checkPE(out(n), true);
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
    'a C program: two translation units, the C library (the compiler\'s, no msvcrt.dll)': () => {
      const [a, b] = platformC('windows');
      const c = build(a, 'c', [b]);
      if (c.status !== 0) return `build failed:\n${c.stdout}${c.stderr}`;
      const p = checkPE(out('c'));
      if (p || !wine) return p;
      const r = run('c', ['x']);
      return r.status === 3 && r.stdout === 'x 42 2.50\r\nbye\r\n' ? null : `status ${r.status}: ${JSON.stringify(r.stdout)}${r.stderr}`;
    },
    // --c-lib=system: the system's C library is the program's (its headers
    // are not here: a directory of declarations stands in for them). Its
    // functions are msvcrt.dll's, by name; main's return ends the program
    // through its exit, which runs the atexit functions and flushes its
    // streams.
    '--c-lib=system: the names msvcrt.dll\'s, main ending through its exit': () => {
      const inc = path.join(tmp, 'windows-sysinc');
      fs.mkdirSync(inc, { recursive: true });
      fs.writeFileSync(path.join(inc, 'stdio.h'), 'int printf(const char *, ...);\nint puts(const char *);\n');
      fs.writeFileSync(path.join(inc, 'stdlib.h'), 'int atexit(void (*)(void));\nint abs(int);\n');
      const src = path.join(tmp, 'windows-syslib.c');
      fs.writeFileSync(src, '#include <stdio.h>\n#include <stdlib.h>\nstatic void bye(void) { printf("bye\\n"); }\n' +
        'int main(void) {\n    atexit(bye);\n    printf("%d\\n", abs(-42));\n    return 3;\n}\n');
      const c = build(src, 'syslib', ['--c-lib=system', `--c-system-include=${inc}`, '--c-long=32']);
      if (c.status !== 0) return `build failed:\n${c.stdout}${c.stderr}`;
      const r0 = checkPEFile(out('syslib'), {});
      if (r0.errs.length) return r0.errs.join('\n');
      for (const f of ['printf', 'atexit', 'abs', 'exit']) if (!r0.imports.includes(`msvcrt.dll:${f}`)) return `no import msvcrt.dll:${f}: ${r0.imports.join(' ')}`;
      if (!wine) return null;
      const r = run('syslib');
      return r.status === 3 && r.stdout.replace(/\r/g, '') === '42\nbye\n' ? null : `status ${r.status}: ${JSON.stringify(r.stdout)}${r.stderr}`;
    },
    // The function table (.pdata): entries sorted, apart, each in .text with
    // its unwind information, not overlapping the TLS directory; under Wine a
    // stack walk from a C function through its callers, our start-up and the
    // system's code (windows/walk.c).
    'the function table and a stack walk': () => {
      for (const o of ['-O0', '-O1', '-O2']) {
        const c = build(path.join(root, 'cmd/axiom/internal/tests/windows/walk.c'), `walk${o}`, [o, '--pdb=off']);
        if (c.status !== 0) return `build failed:\n${c.stdout}${c.stderr}`;
        const b = fs.readFileSync(out(`walk${o}`));
        const { off, dir } = peLayout(b);
        const ex = dir(3), tls = dir(9);
        if (!ex.size) return `${o}: no function table`;
        if (tls.size && tls.rva < ex.rva + ex.size && ex.rva < tls.rva + tls.size) return `${o}: the TLS directory overlaps the function table`;
        let prev = 0;
        for (let e = off(ex.rva); e < off(ex.rva) + ex.size; e += 12) {
          const s0 = b.readUInt32LE(e), e0 = b.readUInt32LE(e + 4), u = b.readUInt32LE(e + 8);
          if (!(s0 >= prev && e0 > s0)) return `${o}: function table entry [0x${s0.toString(16)}, 0x${e0.toString(16)}) out of order`;
          // (version 1; no flags but UNW_FLAG_CHAININFO: the code after a sibling call's jump)
          if ((b[off(u)] & 7) !== 1 || (b[off(u)] >> 3) & ~4) return `${o}: unwind information ${b[off(u)]} (version, flags) at 0x${s0.toString(16)}`;
          prev = e0;
        }
        if (!wine) continue;
        const r = run(`walk${o}`);
        const m = /leaf (?:0x)?([0-9A-F]+) main (?:0x)?([0-9A-F]+) die (?:0x)?([0-9A-F]+)/i.exec(r.stdout);
        if (!m) return `${o}: status ${r.status}: ${r.stdout}${r.stderr}`;
        const [leaf, main] = [parseInt(m[1], 16), parseInt(m[2], 16)];
        const fr = [...r.stdout.matchAll(/frame (?:0x)?([0-9A-F]+)/gi)].map((x) => parseInt(x[1], 16)).filter((a) => a !== 0);
        const li = fr.findIndex((a) => a > leaf && a < main), mi = fr.findIndex((a) => a > main && a < main + 0x400);
        // (at -O2 leaf and main are inlined into the start-up's call of main)
        if (o !== '-O2' && (li < 0 || mi !== li + 1)) return `${o}: the walk does not go from leaf to main:\n${r.stdout}`;
        if (!fr.some((a) => a >= 0x200000000)) return `${o}: the walk does not reach the system's code:\n${r.stdout}`;
        if (new Set(fr).size !== fr.length) return `${o}: the walk repeats a frame:\n${r.stdout}`;
      }
      return null;
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
      // its structure (pdb.mjs); the functions' names are in it (the
      // publics), and their files (/names)
      const pr = checkPDB(p);
      if (pr.errs.length) return pr.errs.slice(0, 20).join('\n');
      const text = p.toString('latin1');
      return text.includes('examples/pi/pi.pi_digits') && /examples[\\/]pi[\\/]pi\.ax/.test(text) ? null : 'names or files missing';
    },
    // -g: the program database's types and variables (codeview.ax), its
    // structure checked (pdb.mjs); under Wine, windows/symbols.c (with an
    // Axiom function, windows/symbols.ax) reads it
    // back through the system's symbol handler and compares the values of
    // its variables where it says they are (at -O0 all of them, at -O2 each
    // one it locates) with those the program holds.
    'debug symbols: types and variables (windows/symbols.c)': () => {
      for (const o of ['-O0', '-O2']) {
        const n = `symbols${o}`;
        const c = build(path.join(root, 'cmd/axiom/internal/tests/windows/symbols.c'), n,
          [path.join(root, 'cmd/axiom/internal/tests/windows/symbols.ax'), o, '-g']);
        if (c.status !== 0) return `${o}: build failed:\n${c.stdout}${c.stderr}`;
        const p = checkPE(out(n));
        if (p) return `${o}: ${p}`;
        const pr = checkPDB(fs.readFileSync(out(n).replace(/\.exe$/, '.pdb')));
        if (pr.errs.length) return `${o}: ${pr.errs.slice(0, 20).join('\n')}`;
        if (!wine) continue;
        const r = run(n, o === '-O0' ? [] : ['optimized']);
        if (r.status !== 0 || !/^ok \d+ checks, [1-9]\d* values\r?\n$/.test(r.stdout)) return `${o}: status ${r.status}: ${r.stdout}${r.stderr}`;
      }
      return null;
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
    'aarch64-windows: thread-locals and output': () => {
      const src = path.join(tmp, 'windows-hello.ax');
      fs.writeFileSync(src, 'import "std/io" as io;\n\nthreadlocal let mut tl: u64 = 41;\n\nexport fn main() i32 {\n' +
        '    tl += 1;\n    io.stdout().println("hello") catch return 1;\n    return @int_cast(i32, tl) - 42;\n}\n');
      const p = arm(src, 'a-hello', ['-O2']);
      if (p || !onArm64Windows) return p;
      const r = spawnSync(out('a-hello'), [], { encoding: 'utf8' });
      return r.status === 0 && r.stdout === 'hello\n' ? null : `status ${r.status}: ${r.stdout}${r.stderr}`;
    },
    'aarch64-windows: file-scope asm with .seh_ directives (its unwind record, checked against its code)': () => {
      const src = path.join(tmp, 'windows-a-seh.c');
      fs.writeFileSync(src, '__asm__(".text\\n.p2align 2\\n.globl sa\\nsa:\\n.seh_proc sa\\n"\n' +
        '  "stp x29, x30, [sp, #-16]!\\n.seh_save_fplr_x 16\\nmov x29, sp\\n.seh_set_fp\\nsub sp, sp, #32\\n.seh_stackalloc 32\\n"\n' +
        '  "stp x19, x20, [sp, #16]\\n.seh_save_regp x19, 16\\nstr d8, [sp, #8]\\n.seh_save_freg d8, 8\\n.seh_endprologue\\n"\n' +
        '  "blr x0\\n.seh_startepilogue\\nldr d8, [sp, #8]\\n.seh_save_freg d8, 8\\nldp x19, x20, [sp, #16]\\n.seh_save_regp x19, 16\\n"\n' +
        '  "add sp, sp, #32\\n.seh_stackalloc 32\\nmov sp, x29\\n.seh_set_fp\\nldp x29, x30, [sp], #16\\n.seh_save_fplr_x 16\\n.seh_endepilogue\\nret\\n.seh_endproc\\n.size sa, .-sa\\n");\n' +
        'void sa(void (*)(void));\nstatic void cb(void) {}\nint main(void) { sa(cb); return 0; }\n');
      return arm(src, 'a-seh', ['--c-std=gnu17']);
    },
    'aarch64-windows: examples, at -O0, -O1 and -O2': () => {
      for (const e of ['wordcount', 'http_server', 'texmath', 'pi']) {
        for (const o of ['-O0', '-O1', '-O2']) {
          const p = arm(path.join(root, `examples/${e}/${e}.ax`), `a-${e}${o}`, [o]);
          if (p) return `${e} ${o}: ${p}`;
        }
      }
      return null;
    },
    'aarch64-windows: a C program: two translation units, the C library (the compiler\'s, no msvcrt.dll)': () => {
      const [a, b] = platformC('windows-a');
      const p = arm(a, 'a-c', [b]);
      if (p || !onArm64Windows) return p;
      const r = spawnSync(out('a-c'), ['x'], { encoding: 'utf8' });
      return r.status === 3 && r.stdout === 'x 42 2.50\r\nbye\r\n' ? null : `status ${r.status}: ${JSON.stringify(r.stdout)}${r.stderr}`;
    },
    'aarch64-windows: the compiler': () => arm(path.join(root, 'cmd/axiom/src/main.ax'), 'a-axiom', ['-O2']),
    'aarch64-windows: axiom test --no-run': () => {
      const t = out('a-tests');
      const r = spawnSync(compiler, ['test', path.join(root, 'std/str.ax'), '--target=aarch64-windows', `--no-run=${t}`], { encoding: 'utf8', cwd: root });
      if (r.status !== 0) return `axiom test --no-run: status ${r.status}: ${r.stdout}${r.stderr}`;
      const p = checkPE(t, true);
      if (p) return p;
      const q = spawnSync(compiler, ['test', path.join(root, 'std/str.ax'), '--target=aarch64-windows'], { encoding: 'utf8', cwd: root });
      return onArm64Windows || (q.status !== 0 && /E9001/.test(q.stderr)) ? null : `axiom test: status ${q.status}: ${q.stderr}`;
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

// Objects and libraries in the platforms' formats (docs/c.md, "Objects and
// libraries"): each directory of cmd/axiom/internal/tests/c/objfmt/ is a
// test, for macOS and Windows on both processors (or those its targets:
// directive names), where the system's clang compiles for them (and
// ld64.lld, lld-link, llvm-ar, llvm-readobj and llvm-dlltool are there).
// Its files: main.c, built by the compiler with the others; sys_*.c,
// compiled by clang for the target (-O2; they declare what they use) into
// objects given to the build; arch_*.c into a static library (libarch.a, or
// arch.lib for Windows: -L and -larch); dylib_*.c into a macOS dynamic
// library linked by ld64.lld (libdy.dylib, install name @rpath/libdy.dylib:
// -ldy); ours_*.c compiled by this compiler (--emit=obj, with ours-flags:)
// into objects given to the build, which llvm-readobj reads without a
// complaint; ourlib_*.c into a library (--emit=lib: -lours); oursso_*.c
// into a shared library of this compiler (--emit=shared --c-freestanding:
// a DLL given to the build, which the program loads from its directory,
// or a dynamic library, -loursso), checked (pe.mjs, llvm-objdump). The program is
// run under Wine for x86_64-windows and must print the expect: lines; the
// other targets' programs are checked (macho.mjs, pe.mjs). cc_main.c, if
// there is one, is compiled by clang and linked by the system's linkers with
// the ours_ objects and the ourlib_ library (lld-link: with an import
// library of msvcrt.dll for the cc-imports: names, entry main; ld64.lld:
// against nothing, symbols looked up when loaded), and for x86_64-windows
// run, printing the cc-expect: lines. Directives (// comments of main.c):
// expect:, expect-status:, targets:, flags:, ours-flags:, cc-imports:,
// dlltool: <dll> <names> (an import library of that DLL, given to the
// build), wine-dlls: <names> (-l of those DLLs, found in Wine's own
// directory, which -L names).
if (!skipPlatform && !native) {
  const objfmtDir = path.join(root, 'cmd/axiom/internal/tests/c/objfmt');
  const objfmtDirs = fs.existsSync(objfmtDir) ? fs.readdirSync(objfmtDir).filter((n) => fs.statSync(path.join(objfmtDir, n)).isDirectory()).sort().map((n) => path.join(objfmtDir, n)) : [];
  const llvmTool = (n) => [n, `${n}-18`, `/usr/lib/llvm-18/bin/${n}`].find((x) => spawnSync('sh', ['-c', `command -v ${x}`]).status === 0);
  const clang = llvmTool('clang'), ld64 = llvmTool('ld64.lld'), lldLink = llvmTool('lld-link'), llvmAr = llvmTool('llvm-ar');
  const readobj = llvmTool('llvm-readobj'), dlltool = llvmTool('llvm-dlltool');
  const wine = ['wine64', 'wine', '/usr/lib/wine/wine64'].find((w) => spawnSync(w, ['--version'], { encoding: 'utf8' }).status === 0);
  const wineDlls = ['/usr/lib/x86_64-linux-gnu/wine/x86_64-windows', '/usr/lib/wine/x86_64-windows', '/usr/lib64/wine/x86_64-windows'].find((d) => fs.existsSync(path.join(d, 'kernel32.dll')));
  const triples = { 'x86_64-windows': 'x86_64-pc-windows-msvc', 'aarch64-windows': 'aarch64-pc-windows-msvc', 'x86_64-macos': 'x86_64-apple-macos11', 'aarch64-macos': 'arm64-apple-macos11' };
  const objfmtTest = async (dir, t) => {
    if (bootstrapCompiler()) return { skip: true, reason: 'the C front end is the compiler\'s' };
    if (!clang || !llvmAr || !readobj) return { skip: true, reason: 'no clang, llvm-ar or llvm-readobj' };
    const src = fs.readFileSync(path.join(dir, 'main.c'), 'utf8');
    const dv = (name) => [...src.matchAll(new RegExp(`(?:// |/\\* )${name}: ?(.*?)(?: \\*/)?$`, 'gm'))].map((m) => m[1]);
    const words = (name) => dv(name).flatMap((x) => x.trim().split(/\s+/)).filter((x) => x);
    const targets = words('targets');
    if (targets.length && !targets.includes(t)) return { skip: true, reason: `not for ${t}` };
    const win = t.endsWith('windows');
    const names = fs.readdirSync(dir).sort();
    const pick = (re) => names.filter((n) => re.test(n)).map((n) => path.join(dir, n));
    if (pick(/^dylib_.*\.c$/).length && !ld64) return { skip: true, reason: 'no ld64.lld' };
    if ((dv('dlltool').length || dv('cc-imports').length) && !dlltool) return { skip: true, reason: 'no llvm-dlltool' };
    if (words('wine-dlls').length && !wineDlls) return { skip: true, reason: 'no Wine DLLs' };
    const work = path.join(tmp, `objfmt-${path.basename(dir)}-${t}`);
    fs.mkdirSync(work, { recursive: true });
    const run = async (cmd, args, what) => {
      const r = await spawnAsync(cmd, args, { encoding: 'utf8', timeout: compileTimeout, cwd: work });
      return r.status === 0 ? null : `${what} failed:\n${r.stdout}${r.stderr}`;
    };
    const target = `--target=${t}`, triple = `--target=${triples[t]}`, ext = win ? '.obj' : '.o';
    const ccObj = async (f) => {
      const o = path.join(work, path.basename(f, '.c') + ext);
      const p = await run(clang, [triple, '-O2', '-c', f, '-o', o], `clang ${path.basename(f)}`);
      return p ? { p } : { o };
    };
    const inputs = [];
    for (const f of pick(/^sys_.*\.c$/)) {
      const r = await ccObj(f);
      if (r.p) return r.p;
      inputs.push(r.o);
    }
    const libFlags = [`-L${work}`];
    const arch = pick(/^arch_.*\.c$/);
    if (arch.length) {
      const aos = [];
      for (const f of arch) {
        const r = await ccObj(f);
        if (r.p) return r.p;
        aos.push(r.o);
      }
      const lib = path.join(work, win ? 'arch.lib' : 'libarch.a');
      fs.rmSync(lib, { force: true });
      const p = await run(llvmAr, ['rcs', ...(win ? [] : ['--format=darwin']), lib, ...aos], 'llvm-ar');
      if (p) return p;
      libFlags.push('-larch');
    }
    const dyl = pick(/^dylib_.*\.c$/);
    if (dyl.length && !win) {
      const dos = [];
      for (const f of dyl) {
        const r = await ccObj(f);
        if (r.p) return r.p;
        dos.push(r.o);
      }
      const p = await run(ld64, ['-arch', t.startsWith('x86_64') ? 'x86_64' : 'arm64', '-platform_version', 'macos', '11.0', '11.0', '-dylib',
        '-install_name', '@rpath/libdy.dylib', '-undefined', 'dynamic_lookup', '-o', path.join(work, 'libdy.dylib'), ...dos], 'ld64.lld -dylib');
      if (p) return p;
      libFlags.push('-ldy');
    }
    if (win) {
      for (const d of dv('dlltool')) {
        const [dll, ...fns] = d.trim().split(/\s+/);
        const def = path.join(work, `${dll}.def`);
        fs.writeFileSync(def, `LIBRARY ${dll}.dll\nEXPORTS\n${fns.join('\n')}\n`);
        const lib = path.join(work, `imp_${dll}.lib`);
        const p = await run(dlltool, ['-m', t.startsWith('x86_64') ? 'i386:x86-64' : 'arm64', '-d', def, '-l', lib], 'llvm-dlltool');
        if (p) return p;
        inputs.push(lib);
      }
      const wd = words('wine-dlls');
      if (wd.length) libFlags.push(`-L${wineDlls}`, ...wd.map((x) => `-l${x}`));
    }
    const oursFlags = words('ours-flags');
    const ours = [];
    for (const f of pick(/^ours_.*\.c$/)) {
      const o = path.join(work, path.basename(f, '.c') + ext);
      const p = await run(compiler, ['build', f, '--emit=obj', target, ...oursFlags, '-o', o], `compiling ${path.basename(f)} (--emit=obj)`);
      if (p) return p;
      const r = spawnSync(readobj, ['--all', o], { encoding: 'utf8' });
      if (r.status !== 0 || /warning|error/i.test(r.stderr)) return `llvm-readobj on ${path.basename(o)}:\n${r.stderr}`;
      ours.push(o);
    }
    const ourlib = pick(/^ourlib_.*\.c$/);
    const ourlibPath = path.join(work, win ? 'ours.lib' : 'libours.a');
    if (ourlib.length) {
      const p = await run(compiler, ['build', ...ourlib, '--emit=lib', target, ...oursFlags, '-o', ourlibPath], 'compiling the ourlib_ files (--emit=lib)');
      if (p) return p;
      libFlags.push('-lours');
    }
    const oursso = pick(/^oursso_.*\.c$/);
    if (oursso.length) {
      const so = path.join(work, win ? 'oursso.dll' : 'liboursso.dylib');
      const p = await run(compiler, ['build', ...oursso, '--emit=shared', '--c-freestanding', target, ...oursFlags, '-o', so], 'compiling the oursso_ files (--emit=shared)');
      if (p) return p;
      if (win) {
        const r = checkPEFile(so, { arm64: t.startsWith('aarch64') });
        if (r.errs.length) return `the DLL: ${r.errs.join('\n')}`;
        inputs.push(so);
      } else {
        const r = spawnSync(llvmTool('llvm-objdump'), ['--macho', '--all-headers', '--bind', '--rebase', '--exports-trie', so], { encoding: 'utf8' });
        if (r.status !== 0 || /error|warning/i.test(r.stderr)) return `llvm-objdump on the dynamic library:\n${r.stderr}`;
        libFlags.push('-loursso');
      }
    }
    const others = names.filter((n) => n !== 'main.c' && /\.c$/.test(n) && !/^(sys|arch|dylib|ours|ourlib|oursso|cc)_/.test(n)).map((n) => path.join(dir, n));
    const out = path.join(work, win ? 'prog.exe' : 'prog');
    const c = await spawnAsync(compiler, ['build', path.join(dir, 'main.c'), ...others, ...inputs, ...ours, ...libFlags, '-o', out, target,
      ...words('flags'), ...extraFlags.filter((a) => !a.startsWith('--units='))], { encoding: 'utf8', timeout: compileTimeout });
    if (c.status !== 0) return `compilation failed:\n${c.stdout}${c.stderr}`;
    const want = dv('expect').join('\n'), status = dv('expect-status').length ? Number(dv('expect-status')[0]) : 0;
    const checkOut = (f) => {
      const d = fs.readFileSync(f);
      const foreign = pick(/^(sys|arch)_.*\.c$/).length > 0;
      const r = win ? checkPEFile(f, { arm64: t.startsWith('aarch64'), foreign }) : checkMachO(d, { x86_64: t.startsWith('x86_64') });
      return r.errs.length ? r.errs.join('\n') : null;
    };
    const bad = checkOut(out);
    if (bad) return bad;
    for (const n of win ? [] : dv('expect-dylib')) {
      if (!fs.readFileSync(out).includes(Buffer.from(n.trim() + '\0'))) return `the program does not load ${n.trim()}`;
    }
    const wenv = { ...process.env, WINEDEBUG: process.env.WINEDEBUG || '-all' };
    const runs = t === 'x86_64-windows' && wine && !buildOnly;
    if (runs) {
      const r = await spawnAsync(wine, [out], { encoding: 'utf8', timeout: runTimeout, env: wenv, cwd: work });
      const got = r.stdout.split('\n').map((l) => l.replace(/\r$/, ''));
      if (got[got.length - 1] === '') got.pop();
      if (r.status !== status) return { problem: `exit status ${r.status}, expected ${status}; output:\n${got.join('\n')}`, stderr: r.stderr };
      if (want !== got.join('\n')) return { problem: `output mismatch\n--- expected\n${want}\n--- got\n${got.join('\n')}`, stderr: r.stderr };
    }
    // this compiler's objects, linked by the system's linker
    const ccMain = path.join(dir, 'cc_main.c');
    if (fs.existsSync(ccMain) && (win ? lldLink : ld64)) {
      const r = await ccObj(ccMain);
      if (r.p) return r.p;
      const out2 = path.join(work, win ? 'cc-prog.exe' : 'cc-prog');
      if (win) {
        const def = path.join(work, 'msvcrt.def'), imp = path.join(work, 'msvcrt-imp.lib');
        fs.writeFileSync(def, `LIBRARY msvcrt.dll\nEXPORTS\n${words('cc-imports').join('\n')}\n`);
        let p = await run(dlltool, ['-m', t.startsWith('x86_64') ? 'i386:x86-64' : 'arm64', '-d', def, '-l', imp], 'llvm-dlltool');
        if (p) return p;
        p = await run(lldLink, ['/nologo', '/entry:main', '/subsystem:console', `/out:${out2}`, r.o, ...ours, ...(ourlib.length ? [ourlibPath] : []), imp], 'lld-link');
        if (p) return p;
        if (runs) {
          const x = await spawnAsync(wine, [out2], { encoding: 'utf8', timeout: runTimeout, env: wenv, cwd: work });
          const got = x.stdout.split('\n').map((l) => l.replace(/\r$/, ''));
          if (got[got.length - 1] === '') got.pop();
          const want2 = dv('cc-expect').join('\n');
          if (x.status !== 0 || want2 !== got.join('\n')) return { problem: `the system linker's program: exit status ${x.status}\n--- expected\n${want2}\n--- got\n${got.join('\n')}`, stderr: x.stderr };
        }
      } else {
        const p = await run(ld64, ['-arch', t.startsWith('x86_64') ? 'x86_64' : 'arm64', '-platform_version', 'macos', '11.0', '11.0', '-undefined', 'dynamic_lookup',
          '-o', out2, r.o, ...ours, ...(ourlib.length ? [ourlibPath] : [])], 'ld64.lld');
        if (p) return p;
      }
    }
    return null;
  };
  const cases = {};
  for (const d of objfmtDirs) {
    for (const t of Object.keys(triples)) cases[`${path.relative(root, d)} [objfmt ${t}]`] = () => objfmtTest(d, t);
  }
  await section('objfmt', cases, { parallel: true });
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
    'package import paths stay in the package': async () => {
      const c = errorCodes(path.join(pk, 'errors/escape'));
      return c.length === 2 && c.every((x) => x === 'E0218') ? null : c.join();
    },
    'an internal import reported from inside the internal tree': async () => {
      // the message names the tree that may import the module (here not under the working directory)
      const lib = fs.realpathSync(path.join(root, 'cmd/axiom/internal/tests/lib'));
      const c = ph(['check', path.join(root, 'cmd/axiom/internal/tests/errors/import_internal.ax')], path.join(lib, 'internal'));
      const want = `error[E0219]: 'answer.ax' is internal: only modules under '${lib}' may import it`;
      return c.status === 1 && (c.stdout + c.stderr).includes(want) ? null : `status ${c.status}:\n${c.stdout}${c.stderr}`;
    },
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
