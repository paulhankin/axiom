#!/usr/bin/env node
// c_header_names.mjs - checks the namespace of the compiler's standard C
// headers (std/c/include/) against cmd/axiom/internal/tests/c/headers/
// names.txt, which lists, from the standards' synopses, the names each
// header has in each standard and those the standard reserves to it.
//
// For each header and each --c-std (c89 c95 c99 c11 c17 c23) the header is
// preprocessed alone (--emit=c-pp), followed by an #ifdef probe of every
// identifier the headers' text has: the identifiers of the declarations it
// leaves (keywords, attributes and reserved names, those beginning with an
// underscore and an upper-case letter or with two underscores, aside) and
// the macros it defines must be names of that header in that standard, or
// reserved to it (C17 7.1.3; a parameter name in a prototype would break a
// program's macro of that name, 7.1.2p4); and every name the standard
// requires of it must be there. A header of a later standard must be an
// error. With __STDC_WANT_LIB_EXT1__ defined as 1, Annex K's names are
// required and allowed too.
//
//   AXIOM=<compiler> node cmd/axiom/internal/tests/tools/c_header_names.mjs [header.h ...]
//
// Prints the problems and exits with status 1 if there are any.
import { spawn } from 'child_process';
import fs from 'fs';
import os from 'os';
import path from 'path';

const root = path.resolve(path.dirname(new URL(import.meta.url).pathname), '../../../../..');
const compiler = path.resolve(process.env.AXIOM || path.join(root, 'cmd/axiom/build/axiom'));
const incDir = path.join(root, 'std/c/include');
const STDS = ['c89', 'c95', 'c99', 'c11', 'c17', 'c23'];
const rank = (s) => STDS.indexOf(s);

// names.txt
const db = new Map();
let cur = null;
for (const raw of fs.readFileSync(path.join(root, 'cmd/axiom/internal/tests/c/headers/names.txt'), 'utf8').split('\n')) {
  const line = raw.replace(/#.*/, '').trim();
  if (!line) continue;
  let m;
  if ((m = /^\[(\S+)\]\s+(\S+)$/.exec(line))) {
    cur = { since: m[2], names: [], reserve: [], include: [], ext1: [] };
    db.set(m[1], cur);
    continue;
  }
  const words = line.split(/\s+/);
  const range = (r) => {
    const [a, b] = r.split('-');
    if (rank(a) < 0 || (b && rank(b) < 0)) throw new Error(`names.txt: bad standard ${r}`);
    return [rank(a), b ? rank(b) : STDS.length - 1];
  };
  if (words[0] === 'reserve') {
    const [lo, hi] = range(words[1]);
    cur.reserve.push({ lo, hi, re: new RegExp(words.slice(2).join(' ').replace(/^\/|\/$/g, '')) });
  } else if (words[0] === 'include') {
    cur.include.push(words[1]);
  } else if (words[0] === 'ext1') {
    cur.ext1.push(...words.slice(1));
  } else {
    const [lo, hi] = range(words[0]);
    for (const w of words.slice(1)) cur.names.push({ lo, hi, name: w.replace(/^\?/, ''), optional: w.startsWith('?') });
  }
}

// What the header h may and must have in standard s (k: its rank)
function allowed(h, k, ext1, seen = new Set()) {
  const e = db.get(h);
  const may = new Set(), must = new Set(), res = [];
  if (!e || seen.has(h)) return { may, must, res };
  seen.add(h);
  for (const n of e.names) {
    if (k < n.lo || k > n.hi) continue;
    may.add(n.name);
    if (!n.optional) must.add(n.name);
  }
  for (const r of e.reserve) if (k >= r.lo && k <= r.hi) res.push(r.re);
  if (ext1) for (const n of e.ext1) { may.add(n); must.add(n); }
  for (const i of e.include) {
    const o = allowed(i, k, ext1, seen);
    for (const n of o.may) may.add(n);
    for (const n of o.must) must.add(n);
    res.push(...o.res);
  }
  return { may, must, res };
}

const KEYWORDS = [
  'auto break case char const continue default do double else enum extern float for goto if int long register return',
  'short signed sizeof static struct switch typedef union unsigned void volatile while',
  'inline restrict _Bool _Complex _Imaginary _Alignas _Alignof _Atomic _Generic _Noreturn _Static_assert _Thread_local',
  'alignas alignof bool constexpr false nullptr static_assert thread_local true typeof typeof_unqual _BitInt',
].join(' ').split(' ');
const keywordsOf = (k) => new Set(KEYWORDS.filter((w) => {
  if (['inline', 'restrict', '_Bool', '_Complex', '_Imaginary'].includes(w)) return k >= rank('c99');
  if (w.startsWith('_') && k >= rank('c11')) return true;
  if (w.startsWith('_')) return false;
  if (['alignas', 'alignof', 'bool', 'constexpr', 'false', 'nullptr', 'static_assert', 'thread_local', 'true', 'typeof',
    'typeof_unqual'].includes(w)) return k >= rank('c23');
  return true;
}));
const reserved = (n) => /^__/.test(n) || /^_[A-Z]/.test(n);

// The identifiers any header's text has: the macro probe's candidates
const universe = new Set();
const walk = (d) => {
  for (const n of fs.readdirSync(d)) {
    const p = path.join(d, n);
    if (fs.statSync(p).isDirectory()) walk(p);
    else for (const m of fs.readFileSync(p, 'utf8').matchAll(/\b[A-Za-z_][A-Za-z0-9_]*\b/g)) universe.add(m[0]);
  }
};
walk(incDir);
for (const e of db.values()) for (const n of [...e.names.map((x) => x.name), ...e.ext1]) universe.add(n);
const standard = new Set();
for (const e of db.values()) for (const n of [...e.names.map((x) => x.name), ...e.ext1]) standard.add(n);
const probes = [...universe].filter((n) => !reserved(n) || standard.has(n)).map((n) => `#ifdef ${n}\n__AXCHK_MACRO_${n}\n#endif\n`).join('');

const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'axc-names-'));
const run = (args) => new Promise((resolve) => {
  const p = spawn(compiler, args, { stdio: ['ignore', 'pipe', 'pipe'] });
  let out = '', err = '';
  p.stdout.on('data', (d) => { out += d; });
  p.stderr.on('data', (d) => { err += d; });
  p.on('close', (status) => resolve({ status, out, err }));
});

// One header in one standard: the problems found
async function check(h, s, ext1) {
  const k = rank(s);
  const e = db.get(h);
  const f = path.join(tmp, `${h.replace(/\W/g, '_')}_${s}_${ext1 ? 'k' : 'n'}.c`);
  fs.writeFileSync(f, `${ext1 ? '#define __STDC_WANT_LIB_EXT1__ 1\n' : ''}#include <${h}>\n${probes}`);
  const r = await run(['build', '--emit=c-pp', f, `--c-std=${s}`, `--std=${path.join(root, 'std')}`]);
  if (k < rank(e.since)) return r.status !== 0 ? [] : [`${h} ${s}: should be an error before ${e.since}`];
  if (r.status !== 0) return [`${h} ${s}${ext1 ? ' (Annex K)' : ''}: preprocessing failed: ${r.out}${r.err}`];
  // the identifiers of the output, outside literals and [[attributes]]
  const text = r.out.replace(/"(?:[^"\\\n]|\\.)*"|'(?:[^'\\\n]|\\.)*'/g, ' ').replace(/\[\[(?:[^\]]|\](?!\]))*\]\]/g, ' ');
  const kw = keywordsOf(k);
  const { may, must, res } = allowed(h, k, ext1);
  const found = new Set();
  for (const m of text.matchAll(/\b[A-Za-z_][A-Za-z0-9_]*\b/g)) {
    let n = m[0];
    if (n.startsWith('__AXCHK_MACRO_')) n = n.slice(14);
    else if (kw.has(n) || ((reserved(n) || /^_[a-z]/.test(n)) && !may.has(n))) continue;
    found.add(n);
  }
  const problems = [];
  const extra = [...found].filter((n) => !may.has(n) && !kw.has(n) && !res.some((re) => re.test(n))).sort();
  if (extra.length) problems.push(`${h} ${s}${ext1 ? ' (Annex K)' : ''}: names it may not have: ${extra.join(' ')}`);
  const missing = [...must].filter((n) => !found.has(n) && !kw.has(n)).sort();
  if (missing.length) problems.push(`${h} ${s}${ext1 ? ' (Annex K)' : ''}: names it lacks: ${missing.join(' ')}`);
  return problems;
}

const headers = process.argv.slice(2).length ? process.argv.slice(2) : [...db.keys()];
const jobs = [];
for (const h of headers) {
  if (!db.has(h)) { console.log(`${h}: not in names.txt`); process.exitCode = 1; continue; }
  for (const s of STDS) {
    jobs.push([h, s, false]);
    if (db.get(h).ext1.length && rank(s) >= rank(db.get(h).since)) jobs.push([h, s, true]);
  }
}
const problems = [];
let next = 0;
await Promise.all(Array.from({ length: Math.min(4, os.cpus().length) }, async () => {
  while (next < jobs.length) {
    const j = jobs[next++];
    problems.push(...await check(...j));
  }
}));
fs.rmSync(tmp, { recursive: true, force: true });
problems.sort();
for (const p of problems) console.log(p);
if (problems.length) process.exitCode = 1;
else console.log(`${jobs.length} header and standard pairs checked`);
