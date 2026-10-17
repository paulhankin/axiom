#!/usr/bin/env python3
"""Generates std/graphics/qr/testdata/cases.txt: QR code matrices made by
segno (https://github.com/heuer/segno; `pip install segno`), an independent
implementation of ISO/IEC 18004, for std/graphics/qr's tests.

std/graphics/qr chooses its segments itself (the shortest bit stream, by
dynamic programming); this script repeats that choice (segment_text below,
a port of qr.segment_text), checks it against a brute-force search over
every segmentation of short strings, and hands the same segments to segno,
with boost_error=False and the version qr would pick (and with segno's
padding of byte-aligned streams corrected, see _write_padding_bits). Masks are segno's
choice (its penalty scoring) unless a case fixes one.

Each case in cases.txt:

    case KIND ECC MASK ECI HEX       KIND: text (qr.encode_with), binary
                                     (qr.encode_binary) or kanji (one
                                     Segment.kanji of Shift JIS bytes);
                                     MASK: auto or 0-7; ECI: 1 or 0;
                                     HEX: the input bytes
    version V mask M                 what segno made
    #######.....                     V*4+17 rows, '#' dark
"""
import itertools
import os
import random
import sys

import segno
import segno.encoder


def _write_padding_bits(buff, version, length):
    # segno 1.6 pads a stream that already ends on a codeword boundary with a
    # whole zero byte (8 - length % 8 bits) where ISO/IEC 18004 7.4.10 adds
    # nothing (the next codeword is a pad codeword, 0xec): fixed here
    buff.extend([0] * ((8 - (length % 8)) % 8))


segno.encoder.write_padding_bits = _write_padding_bits

root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', 'std', 'graphics', 'qr', 'testdata')

ALNUM = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:'


def count_bits(mode, version):
    g = 0 if version < 10 else (1 if version < 27 else 2)
    return {'numeric': 10 + 2 * g, 'alphanumeric': 9 + 2 * g, 'byte': 8 if g == 0 else 16,
            'kanji': 8 + 2 * g}[mode]


def data_bits(mode, n):
    if mode == 'numeric':
        return n // 3 * 10 + [0, 4, 7][n % 3]
    if mode == 'alphanumeric':
        return n // 2 * 11 + n % 2 * 6
    return n * 8


def seg_bits(segs, version):
    return sum(4 + count_bits(m, version) + data_bits(m, len(d)) for m, d in segs)


def allowed(mode, c):
    if mode == 'numeric':
        return 48 <= c <= 57
    if mode == 'alphanumeric':
        return chr(c) in ALNUM if c < 128 else False
    return True


MODES = ['numeric', 'alphanumeric', 'byte']


def segment_text(text, version):
    """The port of qr.segment_text (text: bytes)."""
    n = len(text)
    if n == 0:
        return []
    INF = float('inf')
    head = [(4 + count_bits(m, version)) * 6 for m in MODES]
    frm = [[0] * 3 for _ in range(n)]
    cost = [INF] * 3
    for i, c in enumerate(text):
        each = [20 if allowed('numeric', c) else INF, 33 if allowed('alphanumeric', c) else INF, 48]
        nxt = [INF] * 3
        for m in range(3):
            if each[m] == INF:
                continue
            if i == 0:
                nxt[m] = head[m] + each[m]
                frm[0][m] = m
                continue
            best, bk = INF, 0
            if cost[m] < INF:
                best, bk = cost[m], m
            for k in range(3):
                if k != m and cost[k] < INF:
                    v = -(-cost[k] // 6) * 6 + head[m]
                    if v < best:
                        best, bk = v, k
            nxt[m] = best + each[m]
            frm[i][m] = bk
        cost = nxt
    ceil6 = lambda x: -(-x // 6) * 6
    m = 2
    for k in range(3):
        if ceil6(cost[k]) < ceil6(cost[m]):
            m = k
    segs = []
    end = n
    for i in range(n - 1, -1, -1):
        prev = frm[i][m]
        if i == 0 or prev != m:
            segs.append((MODES[m], text[i:end]))
            end = i
        m = prev
    return segs[::-1]


def brute_force(text, version):
    """The fewest bits of any segmentation."""
    n = len(text)
    best = None
    for cuts in itertools.product([0, 1], repeat=n - 1):
        pieces, start = [], 0
        for i, c in enumerate(cuts):
            if c:
                pieces.append(text[start:i + 1])
                start = i + 1
        pieces.append(text[start:])
        for modes in itertools.product(MODES, repeat=len(pieces)):
            if all(allowed(m, c) for m, p in zip(modes, pieces) for c in p):
                b = seg_bits(list(zip(modes, pieces)), version)
                if best is None or b < best:
                    best = b
    return best


def check_segmentation():
    rnd = random.Random(18004)
    alphabet = b'0123456789ABC $:ab\xc3'
    for _ in range(150):
        t = bytes(rnd.choice(alphabet) for _ in range(rnd.randint(1, 8)))
        for v in (1, 10, 27):
            got = seg_bits(segment_text(t, v), v)
            want = brute_force(t, v)
            assert got == want, (t, v, got, want)


def capacity(version, ecc):
    return segno.consts.SYMBOL_CAPACITY[version][ecc]


SEGNO_MODE = {'numeric': segno.consts.MODE_NUMERIC, 'alphanumeric': segno.consts.MODE_ALPHANUMERIC,
              'byte': segno.consts.MODE_BYTE}
LEVELS = {'L': segno.consts.ERROR_LEVEL_L, 'M': segno.consts.ERROR_LEVEL_M, 'Q': segno.consts.ERROR_LEVEL_Q,
          'H': segno.consts.ERROR_LEVEL_H}


def text_case(text, ecc, mask=None, eci=True):
    ascii_only = all(c < 128 for c in text)
    use_eci = eci and not ascii_only
    for lo, hi in ((1, 9), (10, 26), (27, 40)):
        segs = segment_text(text, lo)
        bits = seg_bits(segs, lo) + (12 if use_eci else 0)
        for v in range(lo, hi + 1):
            if bits <= capacity(v, LEVELS[ecc]):
                if use_eci:
                    # segno writes the designator before each byte segment:
                    # the same stream only if the first segment is the one
                    assert [m for m, _ in segs].count('byte') == 1 and segs[0][0] == 'byte', segs
                items = [(d, SEGNO_MODE[m], 'utf-8' if m == 'byte' else None) for m, d in segs]
                q = segno.make(items, error=ecc, version=v, mask=mask, eci=use_eci, boost_error=False,
                               micro=False)
                return ('text', ecc, mask, eci, text, q)
    raise ValueError('too long')


def binary_case(data, ecc, mask=None):
    q = segno.make([(data, segno.consts.MODE_BYTE)], error=ecc, mask=mask, boost_error=False, micro=False)
    return ('binary', ecc, mask, False, data, q)


def kanji_case(s, ecc, mask=None):
    q = segno.make([(s, segno.consts.MODE_KANJI)], error=ecc, mask=mask, boost_error=False, micro=False)
    return ('kanji', ecc, mask, False, s.encode('shift_jis'), q)


def main():
    check_segmentation()
    cases = [
        text_case(b'HELLO WORLD', 'Q'),
        text_case(b'HELLO WORLD', 'M'),
        text_case(b'01234567', 'M'),
        text_case(b'8675309', 'L'),
        text_case(b'https://www.example.org/', 'L'),
        text_case(b'Hello, world! 123', 'H'),
        text_case(b'3141592653589793238462643383279502884197169399375105820974944592', 'H'),
        text_case(b'ABCDEF0123456789abcdef', 'Q', mask=3),
        text_case(b'abc 1234567890123 XYZ', 'M'),
        text_case('Grüße aus Zürich, 2024'.encode(), 'M'),
        text_case('こんにちは世界'.encode(), 'Q'),
        text_case('price: 12€'.encode(), 'L', eci=False),
    ]
    for m in range(8):
        cases.append(text_case(b'MASK TEST %d' % m, 'L', mask=m))
    rnd = random.Random(40)
    # every version, by length; all four levels
    for v, ecc in [(2, 'L'), (5, 'Q'), (7, 'M'), (9, 'H'), (10, 'L'), (14, 'M'), (20, 'Q'), (26, 'H'),
                   (27, 'L'), (32, 'M'), (35, 'Q'), (40, 'L'), (40, 'H')]:
        # a text that needs about version v: mixed digits, capitals and lower case
        cap = capacity(v, LEVELS[ecc]) // 8
        n = max(cap - 3 - (v // 10) * 2, 1)
        text = bytes(rnd.choice(b'abcdefghijklmnopqrstuvwxyz ,.!?') for _ in range(n - 8)) + b'12345678'
        cases.append(text_case(text, ecc))
    cases.append(text_case(b'1' * 7089, 'L'))           # the most digits
    cases.append(text_case(b'A' * 1852, 'H'))           # alphanumeric, version 40-H
    cases.append(binary_case(bytes(range(256)), 'M'))
    cases.append(binary_case(b'\x00\xff\x10', 'H', mask=5))
    cases.append(kanji_case('点茗', 'Q'))
    cases.append(kanji_case('漢字モード、日本語テキスト', 'M'))
    os.makedirs(root, exist_ok=True)
    with open(os.path.join(root, 'cases.txt'), 'w') as f:
        f.write('# Generated by tests/internal/tools/gen_qr_tests.py from segno %s. Do not edit.\n' % segno.__version__)
        for kind, ecc, mask, eci, data, q in cases:
            f.write('case %s %s %s %d %s\n' % (kind, ecc, 'auto' if mask is None else mask, 1 if eci else 0,
                                               data.hex()))
            f.write('version %d mask %d\n' % (q.version, q.mask))
            for row in q.matrix:
                f.write(''.join('#' if b else '.' for b in row) + '\n')
    print('%d cases' % len(cases), file=sys.stderr)


if __name__ == '__main__':
    main()
