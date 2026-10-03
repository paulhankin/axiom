// tests/run.mjs - Axiom test runner.
//
// Directives (in comments anywhere in a test file):
//   // expect: <line>          expected output line (in order)
//   // expect-trap             `run` must trap
//   // expect-error: E0412     compilation must fail with this code (repeatable)
//   // flags: --release        extra compiler flags
//   // wasm-only               skip on the native target
//   // native-only             skip on wasm
//   // link: file.c            C source (next to the test) compiled and linked in
//   // header: name.h          generate a C header for the exports (for the C source)
//
// Usage: node tests/run.mjs [--native] [--filter=substr] [-O2] [--units=module|program]
import { spawnSync } from 'child_process';
import fs from 'fs';
import path from 'path';
import zlib from 'zlib';
import os from 'os';
import { Worker } from 'worker_threads';
import { createHost } from '../runtime/host.mjs';
import { checkMachO } from './macho.mjs';

const root = path.resolve(path.dirname(new URL(import.meta.url).pathname), '..');
const compiler = path.resolve(process.env.AXIOM || path.join(root, 'compiler/build/axiom'));
const argv = process.argv.slice(2);
const native = argv.includes('--native');
// Cross testing: AXIOM_NATIVE_TARGET=aarch64 AXIOM_NATIVE_EXEC="qemu-aarch64 -L /usr/aarch64-linux-gnu"
const nativeTarget = `--target=${process.env.AXIOM_NATIVE_TARGET || 'native'}`;
// Windows (AXIOM_NATIVE_TARGET=x86_64-windows AXIOM_NATIVE_EXEC=wine64) and
// macOS link no C code: tests/native_rt.ax replaces tests/native_rt.c, and
// tests that call C are skipped.
const noC = /windows|macos|darwin/.test(process.env.AXIOM_NATIVE_TARGET || '');
const execWrap = (process.env.AXIOM_NATIVE_EXEC || '').split(' ').filter((x) => x);
// Emulated tests run several times slower: give each test a minute.
const testTimeout = execWrap.length ? ['--timeout=60000'] : [];
const spawnExe = (exe, args, opts) => execWrap.length ? spawnSync(execWrap[0], [...execWrap.slice(1), exe, ...args], opts) : spawnSync(exe, args, opts);
const filter = (argv.find(a => a.startsWith('--filter=')) || '').slice(9);
const extraFlags = argv.filter(a => /^-O[0-3]$/.test(a) || /^--units=(module|program)$/.test(a));
const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'axiom-tests-'));

function files(dir) {
  return fs.readdirSync(dir).filter(f => f.endsWith('.ax')).map(f => path.join(dir, f)).sort();
}

function directives(src) {
  const d = { expect: [], trap: false, errors: [], flags: [], wasmOnly: false, nativeOnly: false, links: [], header: null };
  for (const line of src.split('\n')) {
    let m;
    if ((m = line.match(/\/\/ expect: ?(.*)$/))) d.expect.push(m[1]);
    else if (/\/\/ expect-trap\b/.test(line)) d.trap = true;
    else if ((m = line.match(/\/\/ expect-error: (E\d+)/))) d.errors.push(m[1]);
    else if ((m = line.match(/\/\/ flags: (.*)$/))) d.flags.push(...m[1].trim().split(/\s+/));
    else if (/\/\/ wasm-only\b/.test(line)) d.wasmOnly = true;
    else if (/\/\/ native-only\b/.test(line)) d.nativeOnly = true;
    else if ((m = line.match(/\/\/ link: (\S+)/))) d.links.push(m[1]);
    else if ((m = line.match(/\/\/ header: (\S+)/))) d.header = m[1];
  }
  return d;
}

async function runWasm(file) {
  // Output from the test print helpers (env) and from fd 1 (std/io) is
  // collected as one text stream, in order.
  let text = '';
  let mem;
  const dec = new TextDecoder();
  const line = (s) => { text += s + '\n'; };
  const host = createHost({
    write: (fd, bytes) => {
      if (fd === 1) {
        text += dec.decode(bytes);
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
    return { out: lines(), trapped: false };
  } catch (e) {
    if (e instanceof WebAssembly.RuntimeError || e instanceof RangeError) return { out: lines(), trapped: true, msg: e.message };
    throw e;
  } finally {
    host.close();
  }
}

function runNative(exe) {
  const r = spawnExe(exe, [], { encoding: 'utf8' });
  const out = r.stdout.split('\n');
  if (out[out.length - 1] === '') out.pop();
  return { out, trapped: r.status !== 0, msg: r.signal || `exit ${r.status}` };
}

let pass = 0, fail = 0, skip = 0;
const failures = [];
for (const f of [...files(path.join(root, 'tests/run')), ...files(path.join(root, 'tests/errors'))]) {
  const name = path.relative(root, f);
  if (filter && !name.includes(filter)) continue;
  const d = directives(fs.readFileSync(f, 'utf8'));
  if ((native && d.wasmOnly) || (!native && d.nativeOnly)) { skip++; continue; }
  if (native && noC && (d.links.length || /extern "C"/.test(fs.readFileSync(f, 'utf8')))) { skip++; continue; }
  const out = path.join(tmp, path.basename(f, '.ax') + (native ? '' : '.wasm'));
  const args = ['build', f, '--format=agent-json', '-o', out, ...d.flags, ...extraFlags];
  if (native) args.push(nativeTarget, `--link=${path.join(root, noC ? 'tests/native_rt.ax' : 'tests/native_rt.c')}`);
  for (const l of d.links) args.push(`--link=${path.join(path.dirname(f), l)}`);
  if (d.header) args.push(`--emit-header=${path.join(tmp, d.header)}`);
  const t0 = process.hrtime.bigint();
  const c = spawnSync(compiler, args, { encoding: 'utf8' });
  const ms = Number(process.hrtime.bigint() - t0) / 1e6;
  let problem = null;
  let json = null;
  try { json = JSON.parse(c.stdout); } catch { json = null; }
  if (d.errors.length) {
    const codes = json ? json.diagnostics.filter(x => x.severity === 'error').map(x => x.code) : [];
    if (c.status === 0) problem = `expected errors ${d.errors.join(', ')}, but compilation succeeded`;
    else for (const e of d.errors) if (!codes.includes(e)) problem = `expected ${e}, got [${codes.join(', ')}]:\n${c.stdout}${c.stderr}`;
  } else if (c.status !== 0) {
    problem = `compilation failed:\n${c.stdout}${c.stderr}`;
  } else {
    const r = native ? runNative(out) : await runWasm(out);
    if (d.trap && !r.trapped) problem = 'expected a trap, but run completed';
    if (!d.trap && r.trapped) problem = `unexpected trap: ${r.msg}`;
    const want = d.expect.join('\n'), got = r.out.join('\n');
    if (!problem && want !== got) problem = `output mismatch\n--- expected\n${want}\n--- got\n${got}`;
  }
  if (problem) { fail++; failures.push(`FAIL ${name}: ${problem}`); console.log(`FAIL ${name}`); }
  else { pass++; console.log(`ok   ${name} (${ms.toFixed(0)} ms)`); }
}
// `axiom test`: the standard library's own tests must pass, and the
// self-tests in tests/testing/ must produce exactly the declared results.
const testTargets = native ? [nativeTarget] : [];
function dirsUnder(dir) {
  return [dir, ...fs.readdirSync(dir, { withFileTypes: true })
    .filter((e) => e.isDirectory() && e.name !== 'testdata').map((e) => e.name).sort()
    .flatMap((n) => dirsUnder(path.join(dir, n)))];
}
const stdDirs = dirsUnder(path.join(root, 'std'));
const stdFiles = stdDirs.flatMap(files).filter((f) => /^test "/m.test(fs.readFileSync(f, 'utf8')));
for (const f of [...stdFiles, ...files(path.join(root, 'tests/testing'))]) {
  const name = path.relative(root, f) + ' [axiom test]';
  if (filter && !name.includes(filter)) continue;
  const src = fs.readFileSync(f, 'utf8');
  const d = directives(src);
  const t0 = process.hrtime.bigint();
  const c = spawnSync(compiler, ['test', f, '--format=agent-json', ...testTargets, ...testTimeout, ...d.flags, ...extraFlags], { encoding: 'utf8' });
  const ms = Number(process.hrtime.bigint() - t0) / 1e6;
  let problem = null;
  let json = null;
  try { json = JSON.parse(c.stdout); } catch { problem = `bad output (status ${c.status}):\n${c.stdout}${c.stderr}`; }
  if (json) {
    const want = [...src.matchAll(/^\/\/ test-result: (.+?) \| (pass|fail)(?: \| (\S+))?(?: \| (\d+))?$/gm)];
    if (!want.length) {
      if (json.status !== 'pass') problem = `failures:\n${JSON.stringify(json.tests ? json.tests.filter((t) => t.status !== 'pass') : json, null, 1)}`;
    } else {
      for (const [, tname, status, code, line] of want) {
        const t = json.tests.find((x) => x.name === tname);
        if (!t) { problem = `missing result for "${tname}"`; break; }
        if (t.status !== status) { problem = `"${tname}": expected ${status}, got ${t.status} ${JSON.stringify(t.failure)}`; break; }
        if (code && t.failure.code !== code) { problem = `"${tname}": expected code ${code}, got ${t.failure.code}`; break; }
        if (line && t.failure.span.start_line !== Number(line)) { problem = `"${tname}": expected line ${line}, got ${t.failure.span.start_line}`; break; }
      }
      if (!problem && json.tests.length !== want.length) problem = `expected ${want.length} tests, got ${json.tests.length}`;
    }
  }
  if (problem) { fail++; failures.push(`FAIL ${name}: ${problem}`); console.log(`FAIL ${name}`); }
  else { pass++; console.log(`ok   ${name} (${ms.toFixed(0)} ms)`); }
}

// Stack traces (spec §16.4): `// stack:` and `// errors:` lines in
// tests/traces/traces.ax give the expected frames of each failing test, and
// tests/traces/crash.ax must report a stack overflow with its stack.
{
  const f = path.join(root, 'tests/traces/traces.ax');
  const name = 'tests/traces/traces.ax [stack traces]';
  if (!filter || name.includes(filter)) {
    const src = fs.readFileSync(f, 'utf8');
    const c = spawnSync(compiler, ['test', f, '--format=agent-json', ...testTargets, ...testTimeout, ...extraFlags], { encoding: 'utf8' });
    let problem = null;
    let json = null;
    try { json = JSON.parse(c.stdout); } catch { problem = `bad output (status ${c.status}):\n${c.stdout}${c.stderr}`; }
    const render = (frames) => frames.map((x) => (x.repeat ? `~${x.repeat}` : x.omitted ? null : x.line ? `${x.function}@${x.line}` : x.function))
      .filter((x) => x !== null).join(' ');
    for (const [, kind, tname, want] of json ? src.matchAll(/^\/\/ (stack|errors): (.+?) \| (.+)$/gm) : []) {
      const t = json.tests.find((x) => x.name === tname);
      const frames = t && t.failure && (kind === 'stack' ? t.failure.stack : t.failure.error_trace);
      const got = frames ? render(frames) : '(none)';
      if (got !== want.trim()) { problem = `"${tname}" ${kind}:\n  expected ${want.trim()}\n  got      ${got}`; break; }
    }
    if (problem) { fail++; failures.push(`FAIL ${name}: ${problem}`); console.log(`FAIL ${name}`); }
    else { pass++; console.log(`ok   ${name}`); }
  }
  const cname = 'tests/traces/crash.ax [crash report]';
  if (!filter || cname.includes(filter)) {
    const out = path.join(tmp, 'crash' + (native ? '' : '.wasm'));
    const b = spawnSync(compiler, ['build', path.join(root, 'tests/traces/crash.ax'), '-o', out, ...(native ? [nativeTarget] : []), ...extraFlags], { encoding: 'utf8' });
    let problem = b.status !== 0 ? `build failed: ${b.stdout}${b.stderr}` : null;
    if (!problem) {
      const r = native ? spawnExe(out, [], { encoding: 'utf8' }) : spawnSync(process.execPath, [path.join(root, 'runtime/host.mjs'), out], { encoding: 'utf8' });
      const want = [/trap T0008 \(stack exhausted\)/, /stack trace \(innermost first\):/, /at forever \((?:\S*\/)?tests\/traces\/crash\.ax:4:12\)/,
        /the frame above repeats 255 more times/, /\.\.\. \d+ more frames/];
      for (const w of want) if (!w.test(r.stderr)) { problem = `stderr lacks ${w}:\n${r.stderr}`; break; }
      if (!problem && r.status === 0) problem = 'the program exited normally';
    }
    if (problem) { fail++; failures.push(`FAIL ${cname}: ${problem}`); console.log(`FAIL ${cname}`); }
    else { pass++; console.log(`ok   ${cname}`); }
  }
}

// The address sanitizer (spec §16.8): the programs in tests/sanitize/ are
// built with --sanitize=address. Each states its exit status ("// status: N",
// or "trap" for T0012) and lines its stderr must contain ("// stderr: ...");
// "// status: 0" programs must not report anything.
for (const f of files(path.join(root, 'tests/sanitize'))) {
  const name = path.relative(root, f) + ' [--sanitize=address]';
  if (filter && !name.includes(filter)) continue;
  const src = fs.readFileSync(f, 'utf8');
  const st = (src.match(/^\/\/ status: (\w+)/m) || [null, '0'])[1];
  const wants = [...src.matchAll(/^\/\/ stderr: (.+)$/gm)].map((m) => m[1].trim());
  const out = path.join(tmp, 'asan-' + path.basename(f, '.ax') + (native ? '' : '.wasm'));
  const b = spawnSync(compiler, ['build', f, '--sanitize=address', '-o', out, ...(native ? [nativeTarget] : []), ...extraFlags],
    { encoding: 'utf8' });
  let problem = b.status !== 0 ? `build failed: ${b.stdout}${b.stderr}` : null;
  if (!problem) {
    const r = native ? spawnExe(out, [], { encoding: 'utf8' })
      : spawnSync(process.execPath, [path.join(root, 'runtime/host.mjs'), out], { encoding: 'utf8' });
    if (st === 'trap') {
      if (r.status === 0 || !/trap T0012 \(memory error\)/.test(r.stderr)) problem = `expected trap T0012, got status ${r.status}:\n${r.stderr}`;
    } else if (r.status !== Number(st)) {
      problem = `expected status ${st}, got ${r.status} ${r.signal || ''}:\n${r.stderr}`;
    }
    if (!problem && st === '0' && /AddressSanitizer/.test(r.stderr)) problem = `unexpected report:\n${r.stderr}`;
    for (const w of wants) if (!problem && !r.stderr.includes(w)) problem = `stderr lacks "${w}":\n${r.stderr}`;
  }
  if (problem) { fail++; failures.push(`FAIL ${name}: ${problem}`); console.log(`FAIL ${name}`); }
  else { pass++; console.log(`ok   ${name}`); }
}

// Profiling (spec §16.6).
{
  const name = 'tests/profile/hot.ax [axiom profile]';
  if (native && noC) skip++;  // (the profiler is Linux's for now)
  else if (!filter || name.includes(filter)) {
    const folded = path.join(tmp, 'hot.folded');
    const c = spawnSync(compiler, ['profile', path.join(root, 'tests/profile/hot.ax'), '--format=agent-json', `--folded=${folded}`,
      ...testTargets, ...extraFlags], { encoding: 'utf8' });
    let problem = null;
    let j = null;
    try { j = JSON.parse(c.stdout); } catch { problem = `bad output (status ${c.status}):\n${c.stdout}${c.stderr}`; }
    if (j) {
      const fn = (n) => j.functions.find((f) => f.name === n);
      const site = (a, b) => j.call_sites.find((x) => x.caller === a && x.callee === b);
      if (j.schema !== 'axiom-profile/1' || j.samples < 20) problem = `too few samples: ${c.stdout}`;
      else if (!fn('spin') || fn('spin').self_pct < 50) problem = `spin is not the hot function: ${JSON.stringify(j.functions)}`;
      else if (!fn('main') || fn('main').total_pct < 90) problem = `main should be on (almost) every stack: ${JSON.stringify(j.functions)}`;
      else if (!site('work', 'spin') || site('work', 'spin').line !== 18) problem = `missing call site work -> spin: ${JSON.stringify(j.call_sites)}`;
      else if (!/^main;work;spin \d+$/m.test(fs.readFileSync(folded, 'utf8'))) problem = `folded stacks lack main;work;spin:\n${fs.readFileSync(folded, 'utf8')}`;
    }
    if (problem) { fail++; failures.push(`FAIL ${name}: ${problem}`); console.log(`FAIL ${name}`); }
    else { pass++; console.log(`ok   ${name}`); }
  }
}

// Separate compilation (spec §16.8): the object cache is reused and
// invalidated along imports, and cached objects stay correct when the
// program's error codes are renumbered.
{
  const name = 'tests/units [separate compilation]';
  if (!filter || name.includes(filter)) {
    const dir = path.join(tmp, 'units');
    fs.mkdirSync(dir, { recursive: true });
    for (const f of ['lib.ax', 'main.ax']) fs.copyFileSync(path.join(root, 'tests/units', f), path.join(dir, f));
    const env = { ...process.env, AXIOM_CACHE: path.join(tmp, 'units-cache') };
    const out = path.join(tmp, 'units-prog' + (native ? '' : '.wasm'));
    const want = ['42', '3', '9', 'too big', 'TooBig', '5'].join('\n');
    const steps = [
      ['cold build', () => {}, (n, t) => n === t],
      ['no change', () => {}, (n) => n === 0],
      ['new error name renumbers all codes', () => fs.appendFileSync(path.join(dir, 'main.ax'), '\nerror { Aardvark }\n'), (n) => n === 1],
      ['library edit', () => fs.appendFileSync(path.join(dir, 'lib.ax'), '\n// edited\n'), (n) => n === 2],
      // on wasm the thread-locals of the whole program share one block whose
      // layout every module's code uses, so a new one recompiles everything
      ['new thread-local', () => fs.appendFileSync(path.join(dir, 'lib.ax'), '\nthreadlocal let mut more: u8 = 0;\n'),
        (n, t) => native ? n === 2 : n === t],
    ];
    let problem = null;
    for (const [what, edit, ok] of steps) {
      edit();
      const c = spawnSync(compiler, ['build', path.join(dir, 'main.ax'), '-o', out, '--verbose', '--units=module',
        ...(native ? [nativeTarget] : [])], { encoding: 'utf8', env });
      const m = /(\d+) modules, (\d+) compiled/.exec(c.stderr);
      if (c.status !== 0 || !m) { problem = `${what}: build failed:\n${c.stdout}${c.stderr}`; break; }
      if (!ok(Number(m[2]), Number(m[1]))) { problem = `${what}: ${m[0]}`; break; }
      const r = native ? spawnExe(out, [], { encoding: 'utf8' }) : spawnSync(process.execPath, [path.join(root, 'runtime/host.mjs'), out], { encoding: 'utf8' });
      if (r.stdout.trim() !== want) { problem = `${what}: output\n${r.stdout}${r.stderr}`; break; }
    }
    if (problem) { fail++; failures.push(`FAIL ${name}: ${problem}`); console.log(`FAIL ${name}`); }
    else { pass++; console.log(`ok   ${name}`); }
  }
}

// The self-hosted compiler reaches the system only through the host ABI, so
// it must stay buildable for WebAssembly (issue 0006): no libc, no 64-bit
// usize assumptions.
{
  const name = 'selfhost/src checks for wasm32';
  if (!filter || name.includes(filter)) {
    const c = spawnSync(compiler, ['check', path.join(root, 'selfhost/src/main.ax'), '--target=wasm32'], { encoding: 'utf8' });
    if (c.status !== 0) { fail++; failures.push(`FAIL ${name}:\n${c.stdout}${c.stderr}`); console.log(`FAIL ${name}`); }
    else { pass++; console.log(`ok   ${name}`); }
  }
}

// A copy of the standard library under the working directory (as when
// testing a checkout of std without --std) is not the library: with the
// address sanitizer, which links the library's std/mem, its modules must not
// clash with the library's in separate compilation.
{
  const name = 'std copy under the working directory [--sanitize=address]';
  if (!filter || name.includes(filter)) {
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
    const c = spawnSync(compiler, ['build', 'std/app.ax', '-o', out, '--sanitize=address', '--units=module',
      ...(native ? [nativeTarget] : [])], { encoding: 'utf8', cwd: dir, env: { ...process.env, AXIOM_CACHE: path.join(tmp, 'stdcopy-cache') } });
    let problem = c.status !== 0 ? `build failed:\n${c.stdout}${c.stderr}` : null;
    if (!problem) {
      const r = native ? spawnExe(out, [], { encoding: 'utf8' }) : spawnSync(process.execPath, [path.join(root, 'runtime/host.mjs'), out], { encoding: 'utf8' });
      if (r.status !== 0) problem = `exit status ${r.status}:\n${r.stderr}`;
    }
    if (problem) { fail++; failures.push(`FAIL ${name}: ${problem}`); console.log(`FAIL ${name}`); }
    else { pass++; console.log(`ok   ${name}`); }
  }
}

// `axiom fmt` (spec §16.5; the self-hosted compiler only). Each
// tests/fmt/<name>.ax formats to <name>.expected, which formats to itself;
// --check lists what is not canonical; a file with a syntax error is left
// alone; the repository's own sources are canonical.
{
  const fdir = path.join(root, 'tests/fmt');
  const fmt = (args) => spawnSync(compiler, ['fmt', ...args], { encoding: 'utf8', cwd: root });
  const probe = fmt(['--check', path.join(fdir, 'layout.expected')]);
  const supported = !/usage:/.test(probe.stderr);
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
    const r = fmt(['--check', 'selfhost/src', 'selfhost/bench', 'std', 'examples', 'runtime', 'tests/run', 'tests/units',
      'tests/lib', 'tests/packages', 'tests/profile']);
    return r.status === 0 ? null : `not canonical:\n${r.stdout}${r.stderr}`;
  };
  for (const [cname, fn] of Object.entries(cases)) {
    const name = `fmt: ${cname}`;
    if (filter && !name.includes(filter)) continue;
    if (!supported) { skip++; continue; }
    const problem = fn();
    if (problem) { fail++; failures.push(`FAIL ${name}: ${problem}`); console.log(`FAIL ${name}`); }
    else { pass++; console.log(`ok   ${name}`); }
  }
}

// macOS (spec §14.1; the self-hosted compiler only). Programs cross-built
// for --target=aarch64-macos cannot run here, so tests/macho.mjs checks the
// executables the way dyld and the kernel would; the compiler itself is
// among them. Unsupported commands say so.
{
  const out = (n) => path.join(tmp, 'macos-' + n);
  const build = (src, n, extra = []) => spawnSync(compiler, ['build', src, '--target=aarch64-macos', '-o', out(n), ...extra],
    { encoding: 'utf8', cwd: root, env: { ...process.env, AXIOM_CACHE: path.join(tmp, 'macos-cache') } });
  const supported = !/unknown target/.test(build(path.join(root, 'examples/wordcount.ax'), 'probe').stderr);
  const check = (src, n, extra) => {
    const c = build(src, n, extra);
    if (c.status !== 0) return `build failed:\n${c.stdout}${c.stderr}`;
    const r = checkMachO(fs.readFileSync(out(n)));
    if (r.errs.length) return r.errs.join('\n');
    return (fs.statSync(out(n)).mode & 0o111) ? null : 'not executable';
  };
  const cases = {
    'thread-locals and output': () => {
      const src = path.join(tmp, 'macos-hello.ax');
      fs.writeFileSync(src, 'import "std/io" as io;\n\nthreadlocal let mut tl: u64 = 41;\n\nexport fn main() i32 {\n' +
        '    tl += 1;\n    io.stdout().println("hello") catch return 1;\n    return @int_cast(i32, tl) - 42;\n}\n');
      return check(src, 'hello', ['-O2']);
    },
    'examples': () => {
      for (const e of ['wordcount', 'http_server', 'texmath']) {
        const p = check(path.join(root, `examples/${e}.ax`), e);
        if (p) return `${e}: ${p}`;
      }
      return null;
    },
    'the compiler': () => check(path.join(root, 'selfhost/src/main.ax'), 'axiom', ['-O1']),
    'unsupported commands': () => {
      const r = spawnSync(compiler, ['test', path.join(root, 'std/str.ax'), '--target=aarch64-macos'], { encoding: 'utf8', cwd: root });
      return r.status !== 0 && /E9001/.test(r.stderr) ? null : `axiom test: status ${r.status}: ${r.stderr}`;
    },
  };
  for (const [cname, fn] of Object.entries(cases)) {
    const name = `macos: ${cname}`;
    if (filter && !name.includes(filter)) continue;
    if (!supported) { skip++; continue; }
    const problem = fn();
    if (problem) { fail++; failures.push(`FAIL ${name}: ${problem}`); console.log(`FAIL ${name}`); }
    else { pass++; console.log(`ok   ${name}`); }
  }
}

// Windows (spec §14.1; the self-hosted compiler only). Programs and the
// compiler itself are cross-built for --target=x86_64-windows and their PE
// headers checked; with Wine installed they also run (the whole suite runs
// on Windows with AXIOM_NATIVE_TARGET=x86_64-windows AXIOM_NATIVE_EXEC=wine64).
{
  const out = (n) => path.join(tmp, `windows-${n}.exe`);
  const env = { ...process.env, AXIOM_CACHE: path.join(tmp, 'windows-cache'), WINEDEBUG: process.env.WINEDEBUG || '-all' };
  const build = (src, n, extra = []) => spawnSync(compiler, ['build', src, '--target=x86_64-windows', '-o', out(n), ...extra],
    { encoding: 'utf8', cwd: root, env });
  const supported = !/unknown target/.test(build(path.join(root, 'examples/wordcount.ax'), 'probe').stderr);
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
  const cases = {
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
    'pi to 2000 places': () => {
      const c = build(path.join(root, 'examples/pi.ax'), 'pi', ['-O2']);
      if (c.status !== 0) return `build failed:\n${c.stdout}${c.stderr}`;
      const p = checkPE(out('pi'));
      if (p || !wine) return p;
      const r = run('pi', ['-p', '-n', '100', path.join(tmp, 'windows-pi.pdf')]);
      const want = '3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679';
      if (r.status !== 0 || !r.stdout.startsWith(want)) return `status ${r.status}: ${r.stdout}${r.stderr}`;
      return fs.readFileSync(path.join(tmp, 'windows-pi.pdf')).subarray(0, 5).toString() === '%PDF-' ? null : 'no PDF';
    },
    'Direct3D 11 and Direct2D': () => {
      const c = build(path.join(root, 'examples/directx.ax'), 'directx', ['-O2']);
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
      const c = build(path.join(root, 'examples/game.ax'), 'game', ['-O2']);
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
    'the compiler': () => {
      const c = build(path.join(root, 'selfhost/src/main.ax'), 'axiom', ['-O1']);
      if (c.status !== 0) return `build failed:\n${c.stdout}${c.stderr}`;
      const p = checkPE(out('axiom'));
      if (p || !wine) return p;
      // it builds a program that runs
      const b = run('axiom', ['build', 'examples/wordcount.ax', '--std=std', '--target=x86_64-windows', '-o', out('wc')]);
      if (b.status !== 0) return `the Windows compiler failed: ${b.stdout}${b.stderr}`;
      const r = run('wc', ['README.md']);
      return r.status === 0 ? null : `wordcount: status ${r.status}: ${r.stdout}${r.stderr}`;
    },
  };
  for (const [cname, fn] of Object.entries(cases)) {
    const name = `windows: ${cname}`;
    if (filter && !name.includes(filter)) continue;
    if (!supported) { skip++; continue; }
    const problem = fn();
    if (problem) { fail++; failures.push(`FAIL ${name}: ${problem}`); console.log(`FAIL ${name}`); }
    else { pass++; console.log(`ok   ${name}${wine ? '' : ' (not run: no Wine)'}`); }
  }
}

// Packages (spec §4.6): builds from a package directory, dependency
// visibility, name conflicts, manifests, git dependencies, `axiom new`.
{
  const pk = path.join(root, 'tests/packages');
  const env = { ...process.env, AXIOM_CACHE: path.join(tmp, 'cache') };
  const ph = (args, cwd) => spawnSync(compiler, [...args, ...extraFlags], { encoding: 'utf8', cwd: cwd || root, env });
  const git = (args, cwd) => spawnSync('git', args, { encoding: 'utf8', cwd,
    env: { ...env, GIT_AUTHOR_NAME: 't', GIT_AUTHOR_EMAIL: 't@t', GIT_COMMITTER_NAME: 't', GIT_COMMITTER_EMAIL: 't@t' } });
  const tgt = native ? [nativeTarget] : [];
  async function buildRun(dir, cwd) {
    const out = path.join(tmp, 'pkg-' + path.basename(dir) + (native ? '' : '.wasm'));
    const c = ph(['build', dir, '--format=agent-json', '-o', out, ...tgt], cwd);
    if (c.status !== 0) return { problem: `compilation failed:\n${c.stdout}${c.stderr}` };
    const r = native ? runNative(out) : await runWasm(out);
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
  const cases = {
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
    'invalid manifest': async () => {
      const c = errorCodes(path.join(pk, 'errors/badmanifest'));
      return c.length === 3 && c.every((x) => x === 'E0215') ? null : c.join();
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
      return codes.includes('E0216') ? null : `bad rev: ${codes.join()}`;
    },
    'axiom new': async () => {
      const c = ph(['new', 'fresh'], tmp);
      if (c.status !== 0) return `status ${c.status}: ${c.stdout}${c.stderr}${c.error || ''}`;
      const j = tested('.', path.join(tmp, 'fresh'));
      return j.status === 'pass' && names(j) === 'add' ? null : JSON.stringify(j);
    },
  };
  for (const [cname, fn] of Object.entries(cases)) {
    const name = `packages: ${cname}`;
    if (filter && !name.includes(filter)) continue;
    const t0 = process.hrtime.bigint();
    const problem = await fn();
    const ms = Number(process.hrtime.bigint() - t0) / 1e6;
    if (problem) { fail++; failures.push(`FAIL ${name}: ${problem}`); console.log(`FAIL ${name}`); }
    else { pass++; console.log(`ok   ${name} (${ms.toFixed(0)} ms)`); }
  }
}

// The node host itself (runtime/host.mjs), driven without a program.
if (!native) {
  const cases = {
    // A socket request completes with a store and then a notify (see
    // runtime/net_worker.mjs). A thread that sees the store before it sleeps
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
  };
  for (const [cname, fn] of Object.entries(cases)) {
    const name = `host: ${cname}`;
    if (filter && !name.includes(filter)) continue;
    const t0 = process.hrtime.bigint();
    const problem = await fn();
    const ms = Number(process.hrtime.bigint() - t0) / 1e6;
    if (problem) { fail++; failures.push(`FAIL ${name}: ${problem}`); console.log(`FAIL ${name}`); }
    else { pass++; console.log(`ok   ${name} (${ms.toFixed(0)} ms)`); }
  }
}

for (const f of failures) console.log('\n' + f);
console.log(`\n${pass} passed, ${fail} failed${skip ? `, ${skip} skipped` : ''}`);
fs.rmSync(tmp, { recursive: true, force: true });
process.exit(fail ? 1 : 0);
