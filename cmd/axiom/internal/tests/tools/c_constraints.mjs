#!/usr/bin/env node
// c_constraints.mjs - the Constraints paragraphs of the C standards' clause
// 6 (and the hosted ones of 5.1.2.2.1), each violated by a program of
// cmd/axiom/internal/tests/c/constraints.txt that `axiom check` must reject
// with the diagnostic listed (5.1.1.3: a constraint violation is
// diagnosed), and programs at the edge of a constraint that it must
// accept. Entries:
//
//   @ 6.5.2.2p2 what the paragraph asks        a clause and paragraph
//   ! C3023 [--c-std=... flags]               rejected with this code
//   + [flags]                                 accepted, without errors
//   <the program, up to the next @, ! or +>
//
//   AXIOM=<compiler> node cmd/axiom/internal/tests/tools/c_constraints.mjs [--list] [filter]
//
// Prints the failures (and with --list every entry's clause) and exits with
// status 1 if there are any.
import { spawn } from 'child_process';
import fs from 'fs';
import os from 'os';
import path from 'path';

const root = path.resolve(path.dirname(new URL(import.meta.url).pathname), '../../../../..');
const compiler = path.resolve(process.env.AXIOM || path.join(root, 'cmd/axiom/build/axiom'));
const args = process.argv.slice(2);
const list = args.includes('--list');
const filter = args.find((a) => !a.startsWith('--')) || '';

const cases = [];
let clause = '', what = '', cur = null;
for (const raw of fs.readFileSync(path.join(root, 'cmd/axiom/internal/tests/c/constraints.txt'), 'utf8').split('\n')) {
  let m;
  if (raw.startsWith('#') && cur === null) continue;
  if ((m = /^@ (\S+) (.*)$/.exec(raw))) {
    clause = m[1];
    what = m[2];
    cur = null;
  } else if ((m = /^! (C\d{4})(.*)$/.exec(raw))) {
    cur = { clause, what, code: m[1], flags: m[2].trim().split(/\s+/).filter((x) => x), text: '' };
    cases.push(cur);
  } else if ((m = /^\+(.*)$/.exec(raw))) {
    cur = { clause, what, code: null, flags: m[1].trim().split(/\s+/).filter((x) => x), text: '' };
    cases.push(cur);
  } else if (cur) {
    cur.text += raw + '\n';
  }
}

const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'axc-constraints-'));
const run = (a) => new Promise((resolve) => {
  const p = spawn(compiler, a, { stdio: ['ignore', 'pipe', 'pipe'] });
  let out = '';
  p.stdout.on('data', (d) => { out += d; });
  p.stderr.on('data', (d) => { out += d; });
  p.on('close', (status) => resolve({ status, out }));
});

const todo = cases.filter((c) => !filter || c.clause.startsWith(filter));
const failures = [];
let next = 0;
await Promise.all(Array.from({ length: Math.min(4, os.cpus().length) }, async () => {
  while (next < todo.length) {
    const k = next++;
    const c = todo[k];
    const f = path.join(tmp, `c${k}.c`);
    fs.writeFileSync(f, c.text);
    const r = await run(['check', f, '--format=agent-json', ...c.flags]);
    let codes = [];
    try {
      codes = JSON.parse(r.out).diagnostics.filter((d) => d.severity === 'error').map((d) => d.code);
    } catch { codes = []; }
    const name = `${c.clause} (${c.what})${c.flags.length ? ' ' + c.flags.join(' ') : ''}`;
    if (list) console.log(`${name}: ${c.code || 'accepted'}`);
    if (c.code === null && r.status !== 0) failures.push(`${name}: rejected: ${r.out.slice(0, 600)}`);
    if (c.code !== null && !codes.includes(c.code))
      failures.push(`${name}: expected ${c.code}, got [${codes.join(', ')}]${codes.length ? '' : ' (accepted)'}:\n${c.text}`);
  }
}));
fs.rmSync(tmp, { recursive: true, force: true });
for (const f of failures.sort()) console.log(`FAIL ${f}`);
const clauses = new Set(todo.map((c) => c.clause));
console.log(`${todo.length} programs, ${clauses.size} paragraphs, ${failures.length} failures`);
if (failures.length) process.exitCode = 1;
