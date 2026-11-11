#!/usr/bin/env python3
"""Writes the C front end's tests of the translation limits (C99 5.2.4.1,
C89 2.2.4.1): a program that reaches each minimum the standard sets and
prints what it computes from it:

    cmd/axiom/internal/tests/tools/gen_c_limits.py

writes cmd/axiom/internal/tests/c/run/limits_c99.c and limits_c89.c, and
cmd/axiom/internal/tests/c/run/limits_inc.h, the header that includes
itself to the nesting depth of #include the standard sets. The expected
output is computed here.
"""
import os

out_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'c', 'run')

# (C99, C89) minimums
L = {
    'nest_blocks': (127, 15),
    'nest_if': (63, 8),
    'declarators': (12, 12),
    'nest_paren_decl': (63, 31),
    'nest_paren_expr': (63, 32),
    'internal_sig': (63, 31),
    'external_ids': (4095, 511),
    'block_ids': (511, 127),
    'macros': (4095, 1024),
    'params': (127, 31),
    'args': (127, 31),
    'macro_params': (127, 31),
    'macro_args': (127, 31),
    'line': (4095, 509),
    'string': (4095, 509),
    'object': (65535, 32767),
    'nest_include': (15, 8),
    'cases': (1023, 257),
    'members': (1023, 127),
    'enumerators': (1023, 127),
    'nest_struct': (63, 15),
}


def chunks(items, sep):
    # (lines of eight items, indented)
    return ['    ' + sep.join(items[i:i + 8]) for i in range(0, len(items), 8)]


def program(c99):
    k = 0 if c99 else 1
    n = {name: v[k] for name, v in L.items()}
    std = 'c99' if c99 else 'c89'
    o = []
    expect = []
    o.append(f'/* flags: --c-std={std} */')
    o.append(f'/* Translation limits ({"C99 5.2.4.1" if c99 else "C89 2.2.4.1"}): each minimum the standard sets is')
    o.append('   reached. Written by cmd/axiom/internal/tests/tools/gen_c_limits.py. */')
    o.append('#include "host.h"')
    # nested includes (the header counts its own depth)
    o.append(f'#define LIMITS_INC_MAX {n["nest_include"]}')
    o.append('#define LIMITS_INC_DEPTH 1')
    o.append('#include "limits_inc.h"')
    # macros, each defined in turn
    for i in range(n['macros']):
        o.append(f'#define M{i} {i}')
    mp = ', '.join(f'a{i}' for i in range(n['macro_params']))
    o.append(f'#define MP({mp}) (a0 + a{n["macro_params"] - 1})')
    # nested conditional inclusion
    o += ['#if 1'] * n['nest_if'] + ['int deep_if = 1;'] + ['#endif'] * n['nest_if']
    # parameters
    ps = ', '.join(f'int p{i}' for i in range(n['params']))
    o.append(f'static int params({ps}) {{ return p0 + p{n["params"] - 1}; }}')
    # members, enumerators
    o.append('struct members {')
    o += chunks([f'char m{i};' for i in range(n['members'])], ' ')
    o.append('};')
    o.append('enum enumerators {')
    o += chunks([f'E{i},' for i in range(n['enumerators'] - 1)] + [f'E{n["enumerators"] - 1}'], ' ')
    o.append('};')
    # nested structure definitions
    s = 'int z;'
    for i in range(n['nest_struct']):
        s = f'struct n{i} {{ {s} }} f{i};'
    o.append(f'struct nested {{ {s} }};')
    # cases
    o.append('static int cases(int x) {')
    o.append('    switch (x) {')
    o += chunks([f'case {i}: return {i * 3};' for i in range(n['cases'])], ' ')
    o.append('    default: return -1;')
    o.append('    }')
    o.append('}')
    # 12 declarators: a pointer to an array of pointers to functions returning ...
    o.append('static int target(void) { return 42; }')
    o.append('static int (*fps[1])(void) = { target };')
    o.append('static int (*(*pfps)[1])(void) = &fps;')
    o.append('static int (*(**ppfps)[1])(void);')
    o.append('static int ' + '*' * (n['declarators'] - 1) + ' *many_pointers;')
    # nested parenthesized declarators
    o.append('static int ' + '(' * n['nest_paren_decl'] + 'paren_decl' + ')' * n['nest_paren_decl'] + ' = 5;')
    # significant initial characters
    base = ('abcdefghij' * 7)[:n['internal_sig'] - 1]
    o.append(f'static int {base}1 = 1;')
    o.append(f'static int {base}2 = 2;')
    # external identifiers
    for i in range(0, n['external_ids'] - 64, 8):
        o.append(' '.join(f'int x{j};' for j in range(i, i + 8)))
    # a long logical line, as a string literal
    lit = 'q' * (n['string'] - 1)
    o.append(f'static const char lit[] = "{lit}";')
    # a large object
    o.append(f'static char object[{n["object"]}];')
    # nested blocks and block-scope identifiers
    o.append('static int blocks(void) {')
    o.append('    int r = 0;')
    o.append('    ' + '{ ' * n['nest_blocks'] + 'r = 7; ' + '} ' * n['nest_blocks'])
    o.append('    return r;')
    o.append('}')
    o.append('static int block_ids(void) {')
    for i in range(0, n['block_ids'], 8):
        o.append('    int ' + ', '.join(f'v{j} = {j}' for j in range(i, min(i + 8, n['block_ids']))) + ';')
    o.append(f'    return v{n["block_ids"] - 1};')
    o.append('}')
    o.append('int main(void) {')
    o.append('    print_i64(limits_inc_deepest);')
    expect.append(n['nest_include'])
    o.append(f'    print_i64(M{n["macros"] - 1});')
    expect.append(n['macros'] - 1)
    ma = ', '.join(str(i) for i in range(n['macro_args']))
    o.append(f'    print_i64(MP({ma}));')
    expect.append(n['macro_args'] - 1)
    o.append('    print_i64(deep_if);')
    expect.append(1)
    args = ', '.join(str(i) for i in range(n['args']))
    o.append(f'    print_i64(params({args}));')
    expect.append(n['args'] - 1)
    o.append('    print_i64(sizeof(struct members));')
    expect.append(n['members'])
    o.append(f'    print_i64(E{n["enumerators"] - 1});')
    expect.append(n['enumerators'] - 1)
    path = '.'.join(f'f{i}' for i in reversed(range(n['nest_struct'])))
    o.append(f'    {{ struct nested v; v.{path}.z = 9; print_i64(v.{path}.z); }}')
    expect.append(9)
    o.append(f'    print_i64(cases({n["cases"] - 1}));')
    expect.append((n['cases'] - 1) * 3)
    o.append('    ppfps = &pfps;')
    o.append('    print_i64((**ppfps)[0]() + (many_pointers == 0));')
    expect.append(43)
    o.append('    print_i64(paren_decl);')
    expect.append(5)
    o.append(f'    print_i64({base}2);')
    expect.append(2)
    o.append(f'    x{n["external_ids"] - 65} = 3;')
    o.append(f'    print_i64(x{n["external_ids"] - 65});')
    expect.append(3)
    o.append('    print_i64(sizeof lit);')
    expect.append(n['string'])
    o.append('    object[sizeof object - 1] = 1;')
    o.append('    print_i64(sizeof object + object[sizeof object - 1]);')
    expect.append(n['object'] + 1)
    o.append('    print_i64(blocks());')
    expect.append(7)
    o.append('    print_i64(block_ids());')
    expect.append(n['block_ids'] - 1)
    e = '(' * n['nest_paren_expr'] + '1' + ')' * n['nest_paren_expr']
    o.append(f'    print_i64({e});')
    expect.append(1)
    o.append('    return 0;')
    o.append('}')
    o.append('')
    o += [f'/* expect: {x} */' for x in expect]
    text = '\n'.join(o) + '\n'
    # (the longest logical line is within the limit)
    assert max(len(x) for x in text.split('\n')) <= n['line'] + 32, std
    return text


HEADER = '''/* limits_inc.h - included by itself to LIMITS_INC_MAX levels (with the file
   that first includes it) by the translation limits tests (limits_c89.c,
   limits_c99.c). Written by cmd/axiom/internal/tests/tools/gen_c_limits.py. */
'''


def header():
    o = [HEADER.rstrip('\n')]
    o.append('#if LIMITS_INC_DEPTH < LIMITS_INC_MAX')
    # LIMITS_INC_DEPTH counts the files open: the includer is 1
    for d in range(1, 15):
        o.append(f'#{"if" if d == 1 else "elif"} LIMITS_INC_DEPTH == {d}')
        o.append('#undef LIMITS_INC_DEPTH')
        o.append(f'#define LIMITS_INC_DEPTH {d + 1}')
    o.append('#endif')
    o.append('#include "limits_inc.h"')
    o.append('#else')
    o.append('static int limits_inc_deepest = LIMITS_INC_DEPTH;')
    o.append('#endif')
    return '\n'.join(o) + '\n'


open(os.path.join(out_dir, 'limits_c99.c'), 'w').write(program(True))
open(os.path.join(out_dir, 'limits_c89.c'), 'w').write(program(False))
open(os.path.join(out_dir, 'limits_inc.h'), 'w').write(header())
