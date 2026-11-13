// cmd/axiom/internal/tests/tools/debug_coverage.mjs - how many of the
// variables of optimized C programs have a location (-g): builds the suite's
// C programs (cmd/axiom/internal/tests/c/prog and c/run, those that build
// for the target) at an optimization level with -g, and counts the
// variables and parameters of their own source files' functions that have
// a location at some address (a location list with an entry, a location
// expression, a constant value), against all of them. Development only
// (not part of the suite).
//
//   AXIOM=<compiler> node cmd/axiom/internal/tests/tools/debug_coverage.mjs
//       [--target=x86_64] [-O2] [--dir=prog|run|all] [--max=N] [--list]
//
// --list prints each variable without a location (file:line name).
import fs from 'fs';
import path from 'path';
import os from 'os';
import { spawnSync } from 'child_process';
import { fileURLToPath } from 'url';
import { loadImage, readDwarf, locList } from '../dwarf.mjs';

const here = path.dirname(fileURLToPath(import.meta.url));
const root = path.resolve(here, '../../../../..');
const argv = process.argv.slice(2);
const opt = (n, d) => { const a = argv.find((x) => x.startsWith(`--${n}=`)); return a ? a.slice(n.length + 3) : d; };
const target = opt('target', 'x86_64');
const level = argv.find((a) => /^-O[0-3]$/.test(a)) || '-O2';
const which = opt('dir', 'all');
const max = Number(opt('max', '100000'));
const list = argv.includes('--list');
const compiler = process.env.AXIOM || path.join(root, 'cmd/axiom/build/axiom');
const tdir = path.join(root, 'cmd/axiom/internal/tests/c');
const tmp = fs.mkdtempSync(path.join(process.env.AXIOM_TMP || os.tmpdir(), 'dbgcov-'));

const sources = [];
for (const d of which === 'all' ? ['prog', 'run'] : [which]) {
  const dir = path.join(tdir, d);
  for (const f of fs.readdirSync(dir).filter((n) => n.endsWith('.c')).sort()) sources.push(path.join(dir, f));
}

const TAG_VAR = 0x34, TAG_PARAM = 0x05, TAG_SUB = 0x2e, TAG_LEX = 0x0b, TAG_INL = 0x1d, AT_INLINE = 0x20, AT_ORIGIN = 0x31;
const AT_NAME = 0x03, AT_LOC = 0x02, AT_CONST = 0x1c, AT_ART = 0x34, AT_FILE = 0x3a, AT_LINE = 0x3b;
let total = 0, located = 0, built = 0;
const missing = [];
for (const f of sources.slice(0, max)) {
  const src = fs.readFileSync(f, 'utf8');
  if (/expect-error|check-only|wasm-only/.test(src)) continue;
  const flags = [...src.matchAll(/\/\* flags: (.*?) \*\//g)].flatMap((m) => m[1].trim().split(/\s+/));
  if (flags.some((x) => x.startsWith('--target=') || x.startsWith('--emit'))) continue;
  const units = [...src.matchAll(/\/\* units: (.*?) \*\//g)].flatMap((m) => m[1].trim().split(/\s+/)).map((u) => path.join(path.dirname(f), u));
  const out = path.join(tmp, 'p');
  const r = spawnSync(compiler, ['build', f, ...units, `-I${tdir}`, `--target=${target}`, level, '-g', ...flags,
    path.join(tdir, 'host_c.c'), '-o', out], { encoding: 'utf8' });
  if (r.status !== 0) continue;
  built++;
  let dw;
  try { dw = readDwarf(loadImage(fs.readFileSync(out))); } catch (e) { console.log(`${path.basename(f)}: ${e.message}`); continue; }
  const base = path.basename(f);
  for (const un of dw.units) {
    // the unit's file names (DW_AT_decl_file indexes the line table's)
    const seq = dw.seqs.find((s) => s.files && s.files.length) || { files: [] };
    const files = seq.files;
    const walk = (die, fn) => {
      for (const c of die.children) {
        if (c.tag === TAG_SUB) {
          if (!c.attrs.has(AT_INLINE)) walk(c, c.attrs.get(AT_NAME));  // (not an abstract instance: no code)
          continue;
        }
        if ((c.tag === TAG_VAR || c.tag === TAG_PARAM) && fn && !c.attrs.has(AT_ART)) {
          // (a variable of an inline instance: its abstract instance's says what it is)
          const o = c.attrs.has(AT_ORIGIN) ? un.dies.get(c.attrs.get(AT_ORIGIN)) : c;
          const fi = o.attrs.get(AT_FILE);
          const fname = files[fi] || '';
          if (!fname.endsWith(base) && !units.some((u) => fname.endsWith(path.basename(u)))) continue;
          // (a block-scope static: always at its address)
          const loc = c.attrs.get(AT_LOC);
          total++;
          let ok = false;
          if (c.attrs.has(AT_CONST)) ok = true;
          else if (loc instanceof Uint8Array) ok = loc.length > 0;
          else if (typeof loc === 'number') ok = locList(dw, loc).length > 0;
          if (ok) located++;
          else missing.push(`${base}:${o.attrs.get(AT_LINE)} ${fn} ${o.attrs.get(AT_NAME)}`);
        }
        if (c.tag === TAG_LEX || c.tag === TAG_INL) walk(c, fn);
      }
    };
    walk(un.die, null);
  }
}
if (list) for (const m of missing) console.log(m);
console.log(`${target} ${level}: ${built} programs, ${located} of ${total} variables with a location (${(100 * located / Math.max(total, 1)).toFixed(1)}%)`);
fs.rmSync(tmp, { recursive: true, force: true });
