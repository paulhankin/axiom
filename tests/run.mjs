// tests/run.mjs - Phalange test runner.
//
// Directives (in comments anywhere in a test file):
//   // expect: <line>          expected output line (in order)
//   // expect-trap             `run` must trap
//   // expect-error: E0412     compilation must fail with this code (repeatable)
//   // flags: --release        extra compiler flags
//   // wasm-only               skip on the native target
//
// Usage: node tests/run.mjs [--native] [--filter=substr] [-O2]
import { spawnSync } from 'child_process';
import fs from 'fs';
import path from 'path';
import os from 'os';
import { createHost } from '../runtime/host.mjs';

const root = path.resolve(path.dirname(new URL(import.meta.url).pathname), '..');
const compiler = process.env.PHALANGE || path.join(root, 'compiler/build/phalange');
const argv = process.argv.slice(2);
const native = argv.includes('--native');
const filter = (argv.find(a => a.startsWith('--filter=')) || '').slice(9);
const extraFlags = argv.filter(a => /^-O[0-3]$/.test(a));
const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'phalange-tests-'));

function files(dir) {
  return fs.readdirSync(dir).filter(f => f.endsWith('.ph')).map(f => path.join(dir, f)).sort();
}

function directives(src) {
  const d = { expect: [], trap: false, errors: [], flags: [], wasmOnly: false };
  for (const line of src.split('\n')) {
    let m;
    if ((m = line.match(/\/\/ expect: ?(.*)$/))) d.expect.push(m[1]);
    else if (/\/\/ expect-trap\b/.test(line)) d.trap = true;
    else if ((m = line.match(/\/\/ expect-error: (E\d+)/))) d.errors.push(m[1]);
    else if ((m = line.match(/\/\/ flags: (.*)$/))) d.flags.push(...m[1].trim().split(/\s+/));
    else if (/\/\/ wasm-only\b/.test(line)) d.wasmOnly = true;
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
  const { instance } = await WebAssembly.instantiate(fs.readFileSync(file), { env, ...host.imports });
  mem = instance.exports.memory;
  host.setMemory(mem);
  const lines = () => { const o = text.split('\n'); if (o[o.length - 1] === '') o.pop(); return o; };
  try {
    instance.exports.run();
    return { out: lines(), trapped: false };
  } catch (e) {
    if (e instanceof WebAssembly.RuntimeError || e instanceof RangeError) return { out: lines(), trapped: true, msg: e.message };
    throw e;
  } finally {
    host.close();
  }
}

function runNative(exe) {
  const r = spawnSync(exe, [], { encoding: 'utf8' });
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
  if (native && d.wasmOnly) { skip++; continue; }
  const out = path.join(tmp, path.basename(f, '.ph') + (native ? '' : '.wasm'));
  const args = ['build', f, '--format=agent-json', '-o', out, ...d.flags, ...extraFlags];
  if (native) args.push('--target=native', `--link=${path.join(root, 'tests/native_rt.c')}`);
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
for (const f of failures) console.log('\n' + f);
console.log(`\n${pass} passed, ${fail} failed${skip ? `, ${skip} skipped` : ''}`);
fs.rmSync(tmp, { recursive: true, force: true });
process.exit(fail ? 1 : 0);
