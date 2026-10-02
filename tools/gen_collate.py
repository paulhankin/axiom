#!/usr/bin/env python3
"""Generates std/unicode/collate/tables.ax: the Default Unicode Collation
Element Table (DUCET) and the data for implicit weights, for
std/unicode/collate (UTS #10).

    python3 tools/gen_collate.py [UCD directory]   # default /usr/share/unicode

It reads allkeys.txt (the DUCET, from the unicode-data package or
https://www.unicode.org/Public/UCA/), PropList.txt (Unified_Ideograph),
Blocks.txt and UnicodeData.txt (canonical decompositions: characters that
have one are never looked up, since collation starts from NFD).

A collation element (CE) is packed in a u32: primary << 15 | secondary << 6
| tertiary << 1 | variable. Single code points go through a two-stage
table: STAGE1[cp >> 8] picks a 256-entry block of STAGE2, whose entries are
0 (no entry: an implicit weight), 0x80000000 | CE (exactly one CE), or
pool index << 8 | count << 1 | starts-a-contraction (count CEs in POOL).
Contractions (2 or 3 code points) are listed by first code point, longest
first, with their code points packed 21 bits each.
"""
import os
import re
import sys

UCD = sys.argv[1] if len(sys.argv) > 1 else '/usr/share/unicode'
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'std', 'unicode', 'collate', 'tables.ax')


def pack(ce):
    var, p, s, t = ce
    assert p < 1 << 16 and s < 1 << 9 and t < 1 << 5
    return (p << 15) | (s << 6) | (t << 1) | (1 if var else 0)


def main():
    version = None
    implicit = []  # (lo, hi, base)
    singles = {}
    contractions = {}
    for line in open(os.path.join(UCD, 'allkeys.txt'), encoding='utf-8'):
        line = line.split('#')[0].strip()
        if not line:
            continue
        if line.startswith('@version'):
            version = line.split()[1]
            continue
        if line.startswith('@implicitweights'):
            r, base = line[len('@implicitweights'):].split(';')
            lo, hi = r.strip().split('..')
            implicit.append((int(lo, 16), int(hi, 16), int(base.strip(), 16)))
            continue
        k, v = line.split(';')
        cps = tuple(int(x, 16) for x in k.split())
        ces = [(f == '*', int(p, 16), int(s, 16), int(t, 16))
               for f, p, s, t in re.findall(r'\[([.*])([0-9A-F]{4})\.([0-9A-F]{4})\.([0-9A-F]{4})\]', v)]
        if len(cps) == 1:
            singles[cps[0]] = ces
        else:
            contractions[cps] = ces
    decomposable = set()
    for line in open(os.path.join(UCD, 'UnicodeData.txt'), encoding='utf-8'):
        f = line.split(';')
        if f[5] and not f[5].startswith('<'):
            decomposable.add(int(f[0], 16))
    starters = {k[0] for k in contractions}
    pool = []
    entries = {}
    for cp, ces in singles.items():
        if cp in decomposable and cp not in starters:
            continue
        if len(ces) == 1 and cp not in starters:
            entries[cp] = 0x80000000 | pack(ces[0])
        else:
            entries[cp] = (len(pool) << 8) | (len(ces) << 1) | (1 if cp in starters else 0)
            pool.extend(pack(c) for c in ces)
    assert len(pool) < 1 << 24
    blocks = sorted({cp >> 8 for cp in entries})
    stage1 = [0] * (0x110000 >> 8)
    stage2 = [0] * 256  # block 0: empty
    for b in blocks:
        stage1[b] = len(stage2) // 256
        stage2.extend(entries.get((b << 8) | i, 0) for i in range(256))
    assert len(stage2) // 256 < 256
    contr = sorted(contractions.items(), key=lambda kv: (kv[0][0], -len(kv[0]), kv[0]))
    c_first, c_seq, c_len, c_ce = [], [], [], []
    for cps, ces in contr:
        seq = 0
        for c in cps:
            seq = (seq << 21) | c
        c_first.append(cps[0])
        c_seq.append(seq)
        c_len.append(len(cps))
        c_ce.append((len(pool) << 8) | (len(ces) << 1))
        pool.extend(pack(c) for c in ces)
    # Unified_Ideograph: core (the two CJK blocks) and the rest
    ideo = []
    for line in open(os.path.join(UCD, 'PropList.txt'), encoding='utf-8'):
        line = line.split('#')[0].strip()
        if not line:
            continue
        r, prop = [x.strip() for x in line.split(';')]
        if prop != 'Unified_Ideograph':
            continue
        lo, _, hi = r.partition('..')
        ideo.append((int(lo, 16), int(hi or lo, 16)))
    core_blocks = []
    for line in open(os.path.join(UCD, 'Blocks.txt'), encoding='utf-8'):
        line = line.split('#')[0].strip()
        if not line:
            continue
        r, name = [x.strip() for x in line.split(';')]
        if name in ('CJK Unified Ideographs', 'CJK Compatibility Ideographs'):
            lo, hi = r.split('..')
            core_blocks.append((int(lo, 16), int(hi, 16)))
    core, other = [], []
    for lo, hi in sorted(ideo):
        for cp_lo, cp_hi, is_core in split(lo, hi, core_blocks):
            (core if is_core else other).append((cp_lo, cp_hi))

    def arr(name, ty, vals, per=12):
        out = ['pub let %s: [%d]%s = [' % (name, len(vals), ty)]
        for i in range(0, len(vals), per):
            out.append('    ' + ', '.join(str(v) for v in vals[i:i + per]) + ',')
        out.append('];')
        return '\n'.join(out)

    with open(OUT, 'w', encoding='utf-8') as f:
        f.write('// Generated by tools/gen_collate.py from the DUCET (allkeys.txt %s), PropList.txt,\n' % version)
        f.write('// Blocks.txt and UnicodeData.txt. Do not edit.\n\n')
        f.write('/// The UCA version of the DUCET.\npub let VERSION: []u8 = "%s";\n\n' % version)
        f.write(arr('STAGE1', 'u8', stage1, 24) + '\n\n')
        f.write(arr('STAGE2', 'u32', stage2) + '\n\n')
        f.write(arr('POOL', 'u32', pool) + '\n\n')
        f.write(arr('CONTR_FIRST', 'u32', c_first) + '\n\n')
        f.write(arr('CONTR_SEQ', 'u64', c_seq, 6) + '\n\n')
        f.write(arr('CONTR_LEN', 'u8', c_len, 24) + '\n\n')
        f.write(arr('CONTR_CE', 'u32', c_ce) + '\n\n')
        f.write(arr('IMPLICIT_LO', 'u32', [x[0] for x in implicit]) + '\n\n')
        f.write(arr('IMPLICIT_HI', 'u32', [x[1] for x in implicit]) + '\n\n')
        f.write(arr('IMPLICIT_BASE', 'u32', [x[2] for x in implicit]) + '\n\n')
        f.write(arr('IDEO_CORE', 'u32', [v for r in core for v in r]) + '\n\n')
        f.write(arr('IDEO_OTHER', 'u32', [v for r in other for v in r]) + '\n')


def split(lo, hi, blocks):
    """[lo, hi] cut at the core blocks' edges: (lo, hi, in a core block)."""
    out = []
    cp = lo
    while cp <= hi:
        inside = next(((b_lo, b_hi) for b_lo, b_hi in blocks if b_lo <= cp <= b_hi), None)
        if inside:
            end = min(hi, inside[1])
            out.append((cp, end, True))
        else:
            nxt = min([b_lo for b_lo, _ in blocks if b_lo > cp] + [hi + 1])
            end = min(hi, nxt - 1)
            out.append((cp, end, False))
        cp = end + 1
    return out


if __name__ == '__main__':
    main()
