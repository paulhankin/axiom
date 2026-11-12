#!/usr/bin/env node
// gen_c_lib_index.mjs - writes std/c/lib/index, the C library's index: which
// file defines each external name, so that a hosted C program checks and
// compiles only the library's units it uses (cmd/axiom/src/main.ax,
// c_library_units). Its lines:
//
//     D name path    path (under std/c/) defines name: a unit of std/c/lib/
//                    (the names the compiler prints with --emit=c-lib-defs)
//                    or a run-time module of std/c/rt/ (its `export fn`s)
//     N path name    the run-time module path calls the C function name (in
//                    an extern "C" block)
//
//     AXIOM=cmd/axiom/build/axiom node cmd/internal/tools/gen_c_lib_index.mjs
//     node cmd/internal/tools/gen_c_lib_index.mjs --check   (the suite: is it current?)
//
// AXIOM is the compiler that reads the units (default cmd/axiom/build/axiom).
import { spawnSync } from 'child_process';
import fs from 'fs';
import path from 'path';

const root = path.resolve(path.dirname(new URL(import.meta.url).pathname), '../../..');
const std = path.join(root, 'std', 'c');
const compiler = path.resolve(process.env.AXIOM || path.join(root, 'cmd/axiom/build/axiom'));
const check = process.argv.includes('--check');

function walk(dir, ext) {
  if (!fs.existsSync(dir)) return [];
  const out = [];
  for (const n of fs.readdirSync(dir).sort()) {
    const p = path.join(dir, n);
    if (fs.statSync(p).isDirectory()) out.push(...walk(p, ext));
    else if (n.endsWith(ext)) out.push(p);
  }
  return out;
}

const lines = [];
const seen = new Map();
const def = (name, rel) => {
  if (seen.has(name)) throw new Error(`${name} is defined by ${seen.get(name)} and ${rel}`);
  seen.set(name, rel);
  lines.push(`D ${name} ${rel}`);
};
for (const f of walk(path.join(std, 'lib'), '.c')) {
  const rel = path.relative(std, f).split(path.sep).join('/');
  const r = spawnSync(compiler, ['build', '--emit=c-lib-defs', f, `--std=${path.join(root, 'std')}`], { encoding: 'utf8' });
  if (r.status !== 0) throw new Error(`${rel}: ${r.stdout}${r.stderr}`);
  for (const n of r.stdout.split('\n').filter((x) => x)) def(n, rel);
}
for (const f of walk(path.join(std, 'rt'), '.ax')) {
  const rel = path.relative(std, f).split(path.sep).join('/');
  const text = fs.readFileSync(f, 'utf8');
  for (const m of text.matchAll(/^export fn (\w+)/gm)) def(m[1], rel);
  for (const b of text.matchAll(/^extern "C" \{([^}]*)\}/gm)) {
    for (const m of b[1].matchAll(/^\s*fn (\w+)/gm)) lines.push(`N ${rel} ${m[1]}`);
  }
}
const text = `# std/c/lib/index - the C library's index (cmd/internal/tools/gen_c_lib_index.mjs
# writes it; do not edit): D name path, N module name
${lines.join('\n')}
`;
const out = path.join(std, 'lib', 'index');
if (check) {
  const old = fs.existsSync(out) ? fs.readFileSync(out, 'utf8') : '';
  if (old !== text) {
    console.error('std/c/lib/index is not current: run cmd/internal/tools/gen_c_lib_index.mjs');
    process.exit(1);
  }
} else {
  fs.writeFileSync(out, text);
}
