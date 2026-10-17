#!/usr/bin/env python3
"""Bundles the yaml-test-suite into std/encoding/yaml/testdata/yaml-test-suite.txt.

    git clone -b data-2022-01-17 https://github.com/yaml/yaml-test-suite
    cmd/axiom/internal/tests/tools/gen_yaml_tests.py path/to/yaml-test-suite

Each case is a header line `case <id> <error 0|1> <yaml bytes> <event bytes>
<json bytes or -1>`, then the input YAML, the expected event stream and (if
there is one) the expected JSON, each followed by a newline.
"""
import os
import subprocess
import sys

src = sys.argv[1]
out = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', '..', '..', 'std', 'encoding', 'yaml', 'testdata', 'yaml-test-suite.txt')
commit = subprocess.run(['git', '-C', src, 'rev-parse', 'HEAD'], capture_output=True, text=True).stdout.strip()
cases = []
for d in sorted(os.listdir(src)):
    p = os.path.join(src, d)
    if not os.path.isdir(p) or d.startswith('.') or d == 'name' or d == 'tags':
        continue
    if os.path.exists(os.path.join(p, 'in.yaml')):
        cases.append((d, p))
    else:
        for sub in sorted(os.listdir(p)):
            q = os.path.join(p, sub)
            if os.path.exists(os.path.join(q, 'in.yaml')):
                cases.append((d + '/' + sub, q))


def read(p, n):
    f = os.path.join(p, n)
    return open(f, 'rb').read() if os.path.exists(f) else None


os.makedirs(os.path.dirname(out), exist_ok=True)
with open(out, 'wb') as f:
    f.write(b'# yaml-test-suite %s (cmd/axiom/internal/tests/tools/gen_yaml_tests.py)\n' % commit.encode())
    for name, p in cases:
        y = read(p, 'in.yaml')
        ev = read(p, 'test.event') or b''
        js = read(p, 'in.json')
        err = 1 if os.path.exists(os.path.join(p, 'error')) else 0
        f.write(b'case %s %d %d %d %d\n' % (name.encode(), err, len(y), len(ev), -1 if js is None else len(js)))
        f.write(y + b'\n' + ev + b'\n')
        if js is not None:
            f.write(js + b'\n')
print(out, len(cases), 'cases')
