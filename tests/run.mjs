// tests/run.mjs - Phalange test runner.
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
import os from 'os';
import { createHost } from '../runtime/host.mjs';

const root = path.resolve(path.dirname(new URL(import.meta.url).pathname), '..');
const compiler = path.resolve(process.env.PHALANGE || path.join(root, 'compiler/build/phalange'));
const argv = process.argv.slice(2);
const native = argv.includes('--native');
// Cross testing: PHALANGE_NATIVE_TARGET=aarch64 PHALANGE_NATIVE_EXEC="qemu-aarch64 -L /usr/aarch64-linux-gnu"
const nativeTarget = `--target=${process.env.PHALANGE_NATIVE_TARGET || 'native'}`;
const execWrap = (process.env.PHALANGE_NATIVE_EXEC || '').split(' ').filter((x) => x);
const spawnExe = (exe, args, opts) => execWrap.length ? spawnSync(execWrap[0], [...execWrap.slice(1), exe, ...args], opts) : spawnSync(exe, args, opts);
const filter = (argv.find(a => a.startsWith('--filter=')) || '').slice(9);
const extraFlags = argv.filter(a => /^-O[0-3]$/.test(a) || /^--units=(module|program)$/.test(a));
const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'phalange-tests-'));

function files(dir) {
  return fs.readdirSync(dir).filter(f => f.endsWith('.ph')).map(f => path.join(dir, f)).sort();
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
  mem = instance.exports.memory;
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
  const out = path.join(tmp, path.basename(f, '.ph') + (native ? '' : '.wasm'));
  const args = ['build', f, '--format=agent-json', '-o', out, ...d.flags, ...extraFlags];
  if (native) args.push(nativeTarget, `--link=${path.join(root, 'tests/native_rt.c')}`);
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
// `phalange test`: the standard library's own tests must pass, and the
// self-tests in tests/testing/ must produce exactly the declared results.
const testTargets = native ? [nativeTarget] : [];
const stdFiles = files(path.join(root, 'std')).filter((f) => /^test "/m.test(fs.readFileSync(f, 'utf8')));
for (const f of [...stdFiles, ...files(path.join(root, 'tests/testing'))]) {
  const name = path.relative(root, f) + ' [phalange test]';
  if (filter && !name.includes(filter)) continue;
  const src = fs.readFileSync(f, 'utf8');
  const d = directives(src);
  const t0 = process.hrtime.bigint();
  const c = spawnSync(compiler, ['test', f, '--format=agent-json', ...testTargets, ...d.flags, ...extraFlags], { encoding: 'utf8' });
  const ms = Number(process.hrtime.bigint() - t0) / 1e6;
  let problem = null;
  let json = null;
  try { json = JSON.parse(c.stdout); } catch { problem = `bad output (status ${c.status}):\n${c.stdout}${c.stderr}`; }
  if (json) {
    const want = [...src.matchAll(/^\/\/ test-result: (.+?) \| (pass|fail)(?: \| (\S+))?(?: \| (\d+))?$/gm)];
    if (!want.length) {
      if (json.status !== 'pass') problem = `failures:\n${JSON.stringify(json.tests.filter((t) => t.status !== 'pass'), null, 1)}`;
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
// tests/traces/traces.ph give the expected frames of each failing test, and
// tests/traces/crash.ph must report a stack overflow with its stack.
{
  const f = path.join(root, 'tests/traces/traces.ph');
  const name = 'tests/traces/traces.ph [stack traces]';
  if (!filter || name.includes(filter)) {
    const src = fs.readFileSync(f, 'utf8');
    const c = spawnSync(compiler, ['test', f, '--format=agent-json', ...testTargets, ...extraFlags], { encoding: 'utf8' });
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
  const cname = 'tests/traces/crash.ph [crash report]';
  if (!filter || cname.includes(filter)) {
    const out = path.join(tmp, 'crash' + (native ? '' : '.wasm'));
    const b = spawnSync(compiler, ['build', path.join(root, 'tests/traces/crash.ph'), '-o', out, ...(native ? [nativeTarget] : []), ...extraFlags], { encoding: 'utf8' });
    let problem = b.status !== 0 ? `build failed: ${b.stdout}${b.stderr}` : null;
    if (!problem) {
      const r = native ? spawnExe(out, [], { encoding: 'utf8' }) : spawnSync(process.execPath, [path.join(root, 'runtime/host.mjs'), out], { encoding: 'utf8' });
      const want = [/trap T0008 \(stack exhausted\)/, /stack trace \(innermost first\):/, /at forever \(tests\/traces\/crash\.ph:4:12\)/,
        /the frame above repeats 255 more times/, /\.\.\. \d+ more frames/];
      for (const w of want) if (!w.test(r.stderr)) { problem = `stderr lacks ${w}:\n${r.stderr}`; break; }
      if (!problem && r.status === 0) problem = 'the program exited normally';
    }
    if (problem) { fail++; failures.push(`FAIL ${cname}: ${problem}`); console.log(`FAIL ${cname}`); }
    else { pass++; console.log(`ok   ${cname}`); }
  }
}

// Profiling (spec §16.6).
{
  const name = 'tests/profile/hot.ph [phalange profile]';
  if (!filter || name.includes(filter)) {
    const folded = path.join(tmp, 'hot.folded');
    const c = spawnSync(compiler, ['profile', path.join(root, 'tests/profile/hot.ph'), '--format=agent-json', `--folded=${folded}`,
      ...testTargets, ...extraFlags], { encoding: 'utf8' });
    let problem = null;
    let j = null;
    try { j = JSON.parse(c.stdout); } catch { problem = `bad output (status ${c.status}):\n${c.stdout}${c.stderr}`; }
    if (j) {
      const fn = (n) => j.functions.find((f) => f.name === n);
      const site = (a, b) => j.call_sites.find((x) => x.caller === a && x.callee === b);
      if (j.schema !== 'phalange-profile/1' || j.samples < 20) problem = `too few samples: ${c.stdout}`;
      else if (!fn('spin') || fn('spin').self_pct < 50) problem = `spin is not the hot function: ${JSON.stringify(j.functions)}`;
      else if (!fn('main') || fn('main').total_pct < 90) problem = `main should be on (almost) every stack: ${JSON.stringify(j.functions)}`;
      else if (!site('work', 'spin') || site('work', 'spin').line !== 18) problem = `missing call site work -> spin: ${JSON.stringify(j.call_sites)}`;
      else if (!/^main;work;spin \d+$/m.test(fs.readFileSync(folded, 'utf8'))) problem = `folded stacks lack main;work;spin:\n${fs.readFileSync(folded, 'utf8')}`;
    }
    if (problem) { fail++; failures.push(`FAIL ${name}: ${problem}`); console.log(`FAIL ${name}`); }
    else { pass++; console.log(`ok   ${name}`); }
  }
}

// Separate compilation (spec §16.7): the object cache is reused and
// invalidated along imports, and cached objects stay correct when the
// program's error codes are renumbered.
{
  const name = 'tests/units [separate compilation]';
  if (!filter || name.includes(filter)) {
    const dir = path.join(tmp, 'units');
    fs.mkdirSync(dir, { recursive: true });
    for (const f of ['lib.ph', 'main.ph']) fs.copyFileSync(path.join(root, 'tests/units', f), path.join(dir, f));
    const env = { ...process.env, PHALANGE_CACHE: path.join(tmp, 'units-cache') };
    const out = path.join(tmp, 'units-prog' + (native ? '' : '.wasm'));
    const want = ['42', '3', '9', 'too big', 'TooBig'].join('\n');
    const steps = [
      ['cold build', () => {}, (n, t) => n === t],
      ['no change', () => {}, (n) => n === 0],
      ['new error name renumbers all codes', () => fs.appendFileSync(path.join(dir, 'main.ph'), '\nerror { Aardvark }\n'), (n) => n === 1],
      ['library edit', () => fs.appendFileSync(path.join(dir, 'lib.ph'), '\n// edited\n'), (n) => n === 2],
    ];
    let problem = null;
    for (const [what, edit, ok] of steps) {
      edit();
      const c = spawnSync(compiler, ['build', path.join(dir, 'main.ph'), '-o', out, '--verbose', '--units=module',
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

// Packages (spec §4.6): builds from a package directory, dependency
// visibility, name conflicts, manifests, git dependencies, `phalange new`.
{
  const pk = path.join(root, 'tests/packages');
  const env = { ...process.env, PHALANGE_CACHE: path.join(tmp, 'cache') };
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
      fs.writeFileSync(path.join(app, 'phalange.toml'), manifest(rev));
      fs.writeFileSync(path.join(app, 'src/main.ph'),
        'import "pkg:util" as util;\nimport "std/io" as io;\n\nexport fn main() i32 {\n' +
        '    io.stdout().print_i64(util.clamp(42, 0, 7)) catch return 1;\n    return 0;\n}\n');
      for (let i = 0; i < 2; i++) {  // the second build uses the cache
        const r = await buildRun(app);
        if (r.problem || r.out !== '7') return r.problem || `output: ${r.out}`;
      }
      fs.writeFileSync(path.join(app, 'phalange.toml'), manifest('0'.repeat(40)));
      const codes = errorCodes(app);
      return codes.includes('E0216') ? null : `bad rev: ${codes.join()}`;
    },
    'phalange new': async () => {
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

for (const f of failures) console.log('\n' + f);
console.log(`\n${pass} passed, ${fail} failed${skip ? `, ${skip} skipped` : ''}`);
fs.rmSync(tmp, { recursive: true, force: true });
process.exit(fail ? 1 : 0);
