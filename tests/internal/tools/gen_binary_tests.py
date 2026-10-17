#!/usr/bin/env python3
"""Bundles conformance vectors for std/encoding/msgpack and std/encoding/cbor.

    git clone https://github.com/kawanet/msgpack-test-suite
    git clone https://github.com/cbor/test-vectors
    tests/internal/tools/gen_binary_tests.py path/to/msgpack-test-suite path/to/test-vectors

std/encoding/msgpack/testdata/suite.txt: one case per line, `<expected>\t<hex> <hex>...`
where every encoding decodes to the expected value (the first is the
shortest, which encoding must reproduce). std/encoding/cbor/testdata/appendix_a.txt
(RFC 8949 Appendix A): `<hex>\t<roundtrip 0/1>\t<diagnostic notation>`.

The expected values use a small dump format: nil, true, false, integers
(also integral floats) in decimal, other floats as f:<16 hex digits of the
float64>, s:<string>, b:<hex>, [a,b], {k:v,...}, ext(type,hex) and
ts(sec,nsec).
"""
import json
import math
import os
import struct
import subprocess
import sys

root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', 'std', 'encoding')


def commit(path):
    return subprocess.run(['git', '-C', path, 'rev-parse', 'HEAD'], capture_output=True, text=True).stdout.strip()


def num(x):
    if isinstance(x, float):
        if x == math.floor(x) and abs(x) < 2 ** 63:
            return str(int(x))
        return 'f:' + struct.pack('>d', x).hex()
    return str(x)


def dump(v):
    if v is None:
        return 'nil'
    if v is True:
        return 'true'
    if v is False:
        return 'false'
    if isinstance(v, (int, float)):
        return num(v)
    if isinstance(v, str):
        return 's:' + v
    if isinstance(v, list):
        return '[' + ','.join(dump(x) for x in v) + ']'
    if isinstance(v, dict):
        return '{' + ','.join(dump(k) + ':' + dump(x) for k, x in v.items()) + '}'
    raise ValueError(v)


def hexs(s):
    return s.replace('-', '')


mp = sys.argv[1]
suite = json.load(open(os.path.join(mp, 'dist', 'msgpack-test-suite.json')))
lines = []
for name, cases in suite.items():
    for c in cases:
        if 'bignum' in c:
            want = c['bignum']
        elif 'number' in c:
            want = num(c['number'])
        elif 'nil' in c:
            want = 'nil'
        elif 'bool' in c:
            want = dump(c['bool'])
        elif 'binary' in c:
            want = 'b:' + hexs(c['binary'])
        elif 'string' in c:
            want = dump(c['string'])
        elif 'array' in c:
            want = dump(c['array'])
        elif 'map' in c:
            want = dump(c['map'])
        elif 'timestamp' in c:
            want = 'ts(%d,%d)' % tuple(c['timestamp'])
        elif 'ext' in c:
            want = 'ext(%d,%s)' % (c['ext'][0], hexs(c['ext'][1]))
        else:
            raise ValueError(c)
        lines.append(want + '\t' + ' '.join(hexs(h) for h in c['msgpack']))
os.makedirs(os.path.join(root, 'msgpack', 'testdata'), exist_ok=True)
with open(os.path.join(root, 'msgpack', 'testdata', 'suite.txt'), 'w') as f:
    f.write('# msgpack-test-suite %s (tests/internal/tools/gen_binary_tests.py)\n' % commit(mp))
    f.write('\n'.join(lines) + '\n')
print('msgpack:', len(lines), 'cases')


# CBOR diagnostic notation, as std/encoding/cbor writes it
def diag_float(x):
    if math.isnan(x):
        return 'NaN'
    if math.isinf(x):
        return 'Infinity' if x > 0 else '-Infinity'
    r = repr(x)
    if 'e' in r:
        m, e = r.split('e')
        if m.endswith('.0'):
            m = m[:-2]
        sign = e[0] if e[0] in '+-' else '+'
        return m + 'e' + sign + e.lstrip('+-').lstrip('0')
    return r


def diag(v):
    if v is None:
        return 'null'
    if v is True:
        return 'true'
    if v is False:
        return 'false'
    if isinstance(v, int):
        if v >= 2 ** 64:
            return "2(h'%s')" % v.to_bytes((v.bit_length() + 7) // 8, 'big').hex()
        if v < -2 ** 64:
            n = -1 - v
            return "3(h'%s')" % n.to_bytes((n.bit_length() + 7) // 8, 'big').hex()
        return str(v)
    if isinstance(v, float):
        return diag_float(v)
    if isinstance(v, str):
        return json.dumps(v, ensure_ascii=False)
    if isinstance(v, list):
        return '[' + ', '.join(diag(x) for x in v) + ']'
    if isinstance(v, dict):
        return '{' + ', '.join(diag(k) + ': ' + diag(x) for k, x in v.items()) + '}'
    raise ValueError(v)


cb = sys.argv[2]
vectors = json.load(open(os.path.join(cb, 'appendix_a.json')))
lines = []
for e in vectors:
    if 'diagnostic' in e:
        want = e['diagnostic']
        if want.startswith('(_ '):
            want = "h'" + ''.join(p.strip()[2:-1] for p in want[3:-1].split(',')) + "'"
    else:
        want = diag(e['decoded'])
    lines.append('%s\t%d\t%s' % (e['hex'], 1 if e['roundtrip'] else 0, want))
os.makedirs(os.path.join(root, 'cbor', 'testdata'), exist_ok=True)
with open(os.path.join(root, 'cbor', 'testdata', 'appendix_a.txt'), 'w') as f:
    f.write('# cbor/test-vectors %s (tests/internal/tools/gen_binary_tests.py)\n' % commit(cb))
    f.write('\n'.join(lines) + '\n')
print('cbor:', len(lines), 'cases')
