#!/usr/bin/env python3
"""Generates cmd/axiom/internal/tests/run/checked_forms.ax: checked arithmetic
in the forms other than two values from memory (run/checked_widths.ax has
those), on every integer width, signed and unsigned, at the edges of the
ranges (the least and greatest values, -1, 0 and their neighbours):

- @add_checked, @sub_checked and @mul_checked of a value and a constant (the
  constant on either side of a sub), which combine turns into a compare of
  the value with a bound;
- @mul_sat of two values, a select on the overflow.

The expectations come from exact (Python) arithmetic.

    python3 cmd/axiom/internal/tests/tools/gen_checked_forms.py
    axiom fmt cmd/axiom/internal/tests/run/checked_forms.ax
"""
import math
import os

OUT = os.path.join(os.path.dirname(__file__), '..', 'run', 'checked_forms.ax')
M64 = (1 << 64) - 1


def rng(bits, signed):
    if signed:
        return -(1 << (bits - 1)), (1 << (bits - 1)) - 1
    return 0, (1 << bits) - 1


def values(bits, signed):
    lo, hi = rng(bits, signed)
    root = math.isqrt(hi)
    vs = [0, 1, 2, 7, hi, hi - 1, hi // 2, hi // 2 + 1, root, root + 1]
    if signed:
        vs += [-1, -2, -7, lo, lo + 1, lo // 2, lo // 2 - 1, -root, -root - 1]
    out = []
    for v in vs:
        if lo <= v <= hi and v not in out:
            out.append(v)
    return out


def consts(bits, signed):
    lo, hi = rng(bits, signed)
    if signed:
        return [lo, -1, 0, 7, hi]
    return [0, 1, 7, hi]


def tname(bits, signed):
    return ('i' if signed else 'u') + str(bits)


def lit(v):
    return str(v)


def as_u64(v):
    return v & M64


def mix(h, r):
    return (h * 31 + as_u64(r)) & M64


def exact(op, a, b):
    return {'add': a + b, 'sub': a - b, 'mul': a * b}[op]


def const_fn(op, bits, signed, lines, expects):
    t = tname(bits, signed)
    vs = 'V_' + t.upper()
    lo, hi = rng(bits, signed)
    lines.append(f'fn {op}_{t}_k() void {{')
    lines.append('    let mut h: u64 = 0;')
    lines.append('    let mut n: u64 = 0;')
    h = 0
    n = 0
    for k in consts(bits, signed):
        sides = [False, True] if op == 'sub' else [False]
        for left in sides:
            expr = f'{lit(k)}, {vs}[i]' if left else f'{vs}[i], {lit(k)}'
            lines.append(f'    for i in 0..{vs}.len {{')
            lines.append(f'        let r = @{op}_checked({expr}) catch {{')
            lines.append('            n += 1;')
            lines.append('            h = h *% 31 +% 7;')
            lines.append('            continue;')
            lines.append('        };')
            if signed:
                lines.append('        h = h *% 31 +% @bit_cast(u64, @as(i64, r));')
            else:
                lines.append('        h = h *% 31 +% @as(u64, r);')
            lines.append('    }')
            for x in values(bits, signed):
                r = exact(op, k, x) if left else exact(op, x, k)
                if lo <= r <= hi:
                    h = mix(h, r)
                else:
                    n += 1
                    h = (h * 31 + 7) & M64
    lines.append('    io.uint(n);')
    lines.append('    io.uint(h);')
    lines.append('}')
    lines.append('')
    expects.append((f'{op}_{t}_k', [n, h]))


def sat_fn(bits, signed, lines, expects):
    t = tname(bits, signed)
    vs = 'V_' + t.upper()
    lo, hi = rng(bits, signed)
    lines.append(f'fn mul_sat_{t}() void {{')
    lines.append('    let mut h: u64 = 0;')
    lines.append(f'    for i in 0..{vs}.len {{')
    lines.append(f'        for j in 0..{vs}.len {{')
    lines.append(f'            let r = @mul_sat({vs}[i], {vs}[j]);')
    if signed:
        lines.append('            h = h *% 31 +% @bit_cast(u64, @as(i64, r));')
    else:
        lines.append('            h = h *% 31 +% @as(u64, r);')
    lines.append('        }')
    lines.append('    }')
    lines.append('    io.uint(h);')
    lines.append('}')
    lines.append('')
    h = 0
    for a in values(bits, signed):
        for b in values(bits, signed):
            h = mix(h, max(lo, min(hi, a * b)))
    expects.append((f'mul_sat_{t}', [h]))


def main():
    lines = [
        '// Checked arithmetic of a value and a constant (@add_checked, @sub_checked,',
        '// @mul_checked; the constant on either side of a sub), which combine turns',
        '// into a compare of the value with a bound, and @mul_sat of two values (a',
        '// select on the overflow), on every integer width, signed and unsigned, at',
        '// the edges of the ranges: the number of overflows and a hash of the',
        '// results. Native code takes the overflow from the flags of the operation',
        '// (the branch or the select reads them). The values come from memory, so',
        '// that nothing is folded; the same at every optimization level. (Generated',
        '// by cmd/axiom/internal/tests/tools/gen_checked_forms.py, with the',
        '// expectations from the exact results.)',
        'import "../lib/io.ax" as io;',
        '',
    ]
    types = [(b, s) for b in (8, 16, 32, 64) for s in (False, True)]
    for bits, signed in types:
        t = tname(bits, signed)
        vs = values(bits, signed)
        lines.append(f'let mut V_{t.upper()}: [{len(vs)}]{t} = [{", ".join(lit(v) for v in vs)}];')
    lines.append('')
    expects = []
    for bits, signed in types:
        for op in ('add', 'sub', 'mul'):
            const_fn(op, bits, signed, lines, expects)
        sat_fn(bits, signed, lines, expects)
    lines.append('export fn run() void {')
    for name, outs in expects:
        lines.append(f'    {name}();  // expect: {outs[0]}')
        for o in outs[1:]:
            lines.append(f'    // expect: {o}')
    lines.append('}')
    with open(OUT, 'w') as f:
        f.write('\n'.join(lines) + '\n')


main()
