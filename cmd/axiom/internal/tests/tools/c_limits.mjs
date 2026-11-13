#!/usr/bin/env node
// c_limits.mjs - the translation limits of C17 5.2.4.1 (C23's are the
// same; C89's 2.2.4.1 and C99's smaller or equal), each reached by a
// program generated here, built with the compiler (for wasm32, in each
// standard the limit is the program's) and run with the node host: it
// prints what it computed, which must be what the generator expects.
// Also each limit far exceeded (at least 4 times), where this
// implementation's own limits (docs/c.md, "Translation limits") allow.
//
//   AXIOM=<compiler> node cmd/axiom/internal/tests/tools/c_limits.mjs [name]
import { spawnSync } from 'child_process';
import fs from 'fs';
import os from 'os';
import path from 'path';

const root = path.resolve(path.dirname(new URL(import.meta.url).pathname), '../../../../..');
const compiler = path.resolve(process.env.AXIOM || path.join(root, 'cmd/axiom/build/axiom'));
const host = path.join(root, 'std/internal/runtime/host.mjs');
const only = process.argv[2] || '';
const rep = (n, f, sep = '') => Array.from({ length: n }, (_, i) => f(i)).join(sep);

// Each limit: [name, the standard's figure, scale, generator(n) -> [source, expected output]]
const limits = [
  ['127 nesting levels of blocks', 127, (n) =>
    [`#include <stdio.h>\nint main(void) { int x = 0; ${rep(n, () => '{ x++; ')}${rep(n, () => '}')} printf("%d\\n", x); return 0; }\n`, `${n}`]],
  ['63 nesting levels of conditional inclusion', 63, (n) =>
    [`#include <stdio.h>\n${rep(n, (i) => `#if ${i} >= 0\n`)}#define DEEP ${n}\n${rep(n, () => '#endif\n')}int main(void) { printf("%d\\n", DEEP); return 0; }\n`, `${n}`]],
  ['12 pointer, array and function declarators of one type', 12, (n) =>
    [`#include <stdio.h>\nint ${rep(n, () => '*')}p;\nint main(void) { printf("%d\\n", (int)sizeof p == (int)sizeof(void *)); return 0; }\n`, '1']],
  ['63 nesting levels of parenthesized declarators', 63, (n) =>
    [`#include <stdio.h>\nint ${rep(n, () => '(')}x${rep(n, () => ')')} = 5;\nint main(void) { printf("%d\\n", x); return 0; }\n`, '5']],
  ['63 nesting levels of parenthesized expressions', 63, (n) =>
    [`#include <stdio.h>\nint main(void) { int v = ${rep(n, () => '(1 + ')}0${rep(n, () => ')')}; printf("%d\\n", v); return 0; }\n`, `${n}`]],
  ['63 significant characters of an internal identifier', 63, (n) => {
    const a = 'i' + rep(n - 2, () => 'x') + 'a', b = 'i' + rep(n - 2, () => 'x') + 'b';
    return [`#include <stdio.h>\nstatic int ${a} = 1;\nstatic int ${b} = 2;\n#define M${a.slice(1)} 3\n#define M${b.slice(1)} 4\nint main(void) { printf("%d %d %d %d\\n", ${a}, ${b}, M${a.slice(1)}, M${b.slice(1)}); return 0; }\n`, '1 2 3 4'];
  }],
  ['31 significant characters of an external identifier', 31, (n) => {
    const a = 'e' + rep(n - 2, () => 'y') + 'a', b = 'e' + rep(n - 2, () => 'y') + 'b';
    return [`#include <stdio.h>\nint ${a} = 1;\nint ${b} = 2;\nint main(void) { printf("%d %d\\n", ${a}, ${b}); return 0; }\n`, '1 2'];
  }],
  ['4095 external identifiers in one translation unit', 4095, (n) =>
    [`#include <stdio.h>\n${rep(n - 1, (i) => `int ext${i} = ${i % 7};`, '\n')}\nint main(void) { long s = 0;\n${rep(n - 1, (i) => `s += ext${i};`, '\n')}\nprintf("%ld\\n", s); return 0; }\n`,
      `${rep(n - 1, (i) => i % 7, ',').split(',').reduce((a, b) => a + Number(b), 0)}`]],
  ['511 identifiers with block scope in one block', 511, (n) =>
    [`#include <stdio.h>\nint main(void) { ${rep(n, (i) => `int b${i} = ${i};`, ' ')} long s = 0; ${rep(n, (i) => `s += b${i};`, ' ')} printf("%ld\\n", s); return 0; }\n`, `${(n * (n - 1)) / 2}`]],
  ['4095 macro identifiers defined at once', 4095, (n) =>
    [`#include <stdio.h>\n${rep(n, (i) => `#define MAC${i} ${i % 5}`, '\n')}\nint main(void) { printf("%d %d\\n", MAC0 + MAC${n - 1}, MAC${n >> 1}); return 0; }\n`, `${(n - 1) % 5} ${(n >> 1) % 5}`]],
  ['127 parameters in one function definition, 127 arguments in one call', 127, (n) =>
    [`#include <stdio.h>\nstatic long f(${rep(n, (i) => `long p${i}`, ', ')}) { return ${rep(n, (i) => `p${i} * ${i + 1}`, ' + ')}; }\nint main(void) { printf("%ld\\n", f(${rep(n, (i) => `${i}`, ', ')})); return 0; }\n`,
      `${rep(n, (i) => i * (i + 1), ',').split(',').reduce((a, b) => a + Number(b), 0)}`]],
  ['127 parameters in one macro definition, 127 arguments in one invocation', 127, (n) =>
    [`#include <stdio.h>\n#define F(${rep(n, (i) => `a${i}`, ', ')}) (${rep(n, (i) => `a${i}`, ' + ')})\nint main(void) { printf("%d\\n", F(${rep(n, (i) => `${i}`, ', ')})); return 0; }\n`, `${(n * (n - 1)) / 2}`]],
  ['4095 characters in a logical source line', 4095, (n) => {
    // (declarations of 12 characters each, and spaces, on one line)
    const k = Math.floor((n - 40) / 12);
    const line = `${rep(k, (i) => `int z${String(i).padStart(4, '0')} = 1;`)}int main(void) { return 0; }`;
    return [`${line}${rep(n - line.length, () => ' ')}\n`, ''];
  }],
  ['4095 characters in a string literal (after concatenation)', 4095, (n) =>
    [`#include <stdio.h>\n#include <string.h>\nstatic const char s[] = "${rep(n >> 1, () => 'a')}" "${rep(n - (n >> 1), () => 'b')}";\nint main(void) { printf("%d %d\\n", (int)strlen(s), (int)sizeof s); return 0; }\n`, `${n} ${n + 1}`]],
  ['65535 bytes in an object (hosted)', 65535, (n) =>
    [`#include <stdio.h>\nstatic unsigned char big[${n}];\nint main(void) { int i; long s = 0; for (i = 0; i < ${n}; i++) big[i] = (unsigned char)i; for (i = 0; i < ${n}; i++) s += big[i]; printf("%d %ld\\n", (int)sizeof big, s); return 0; }\n`,
      `${n} ${Array.from({ length: n }, (_, i) => i & 255).reduce((a, b) => a + b, 0)}`]],
  ['15 nesting levels of #included files', 15, null],
  ['1023 case labels in one switch', 1023, (n) =>
    [`#include <stdio.h>\nstatic int f(int v) { switch (v) { ${rep(n, (i) => `case ${i * 3}: return ${i};`, ' ')} } return -1; }\nint main(void) { printf("%d %d %d\\n", f(0), f(${(n - 1) * 3}), f(1)); return 0; }\n`, `0 ${n - 1} -1`]],
  ['1023 members in one structure', 1023, (n) =>
    [`#include <stdio.h>\nstruct big { ${rep(n, (i) => `char m${i};`, ' ')} };\nint main(void) { struct big b; b.m${n - 1} = 7; printf("%d %d\\n", (int)sizeof(struct big), b.m${n - 1}); return 0; }\n`, `${n} 7`]],
  ['1023 enumeration constants in one enumeration', 1023, (n) =>
    [`#include <stdio.h>\nenum e { ${rep(n, (i) => `E${i}`, ', ')} };\nint main(void) { printf("%d\\n", (int)E${n - 1}); return 0; }\n`, `${n - 1}`]],
  ['63 levels of nested structure definitions', 63, (n) =>
    [`#include <stdio.h>\n${rep(n, (i) => `struct s${i} { int v${i}; `)}${rep(n - 1, (i) => `} m${n - 1 - i};`)}};\nint main(void) { struct s0 x; x.m1.m2.v2 = 3; printf("%d\\n", x.m1.m2.v2); return 0; }\n`, '3']],
];

const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'axc-limits-'));
let failures = 0, count = 0;
function check(name, src, want, std, extraFiles = {}) {
  const dir = fs.mkdtempSync(path.join(tmp, 'l'));
  for (const [f, t] of Object.entries(extraFiles)) fs.writeFileSync(path.join(dir, f), t);
  const f = path.join(dir, 'limit.c');
  fs.writeFileSync(f, src);
  const out = path.join(dir, 'limit.wasm');
  count++;
  const b = spawnSync(compiler, ['build', f, `--c-std=${std}`, '-o', out], { encoding: 'utf8' });
  if (b.status !== 0) {
    console.log(`FAIL ${name} (${std}): build failed: ${(b.stdout + b.stderr).slice(0, 800)}`);
    failures++;
    return;
  }
  const r = spawnSync(process.execPath, [host, out], { encoding: 'utf8' });
  if (r.stdout.trim() !== want) {
    console.log(`FAIL ${name} (${std}): printed ${JSON.stringify(r.stdout.trim())}, expected ${JSON.stringify(want)} ${r.stderr}`);
    failures++;
  }
}
for (const [name, n, gen] of limits) {
  if (only && !name.includes(only)) continue;
  if (gen === null) {
    // #include nesting: files i.h including (i+1).h, the last defining DEPTH
    for (const k of [n, 4 * n]) {
      const files = {};
      for (let i = 0; i < k; i++) files[`inc${i}.h`] = i + 1 < k ? `#include "inc${i + 1}.h"\n` : `#define DEPTH ${k}\n`;
      check(`${name} (x${k / n})`, `#include <stdio.h>\n#include "inc0.h"\nint main(void) { printf("%d\\n", DEPTH); return 0; }\n`, `${k}`, 'c17', files);
    }
    continue;
  }
  for (const std of ['c89', 'c17', 'c23']) {
    if (std === 'c89' && /parameter|argument|external identifier|significant|65535/.test(name)) continue;
    const [src, want] = gen(n);
    check(name, std === 'c89' ? src.replace(/\/\/[^\n]*/g, '') : src, want, std);
  }
  // (far beyond: four times the figure, eight for the small ones; blocks
  // twice: this implementation's limit is 1000 levels of expressions,
  // declarators and statements together, of which a block takes more than
  // one, docs/c.md)
  const times = /blocks/.test(name) ? 2 : n < 64 ? 8 : 4;
  const [src, want] = gen(n * times);
  check(`${name} (x${times})`, src, want, 'c17');
}
fs.rmSync(tmp, { recursive: true, force: true });
console.log(`${count} programs, ${failures} failures`);
if (failures) process.exitCode = 1;
