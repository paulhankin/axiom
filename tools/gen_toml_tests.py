#!/usr/bin/env python3
"""Bundles the TOML 1.0.0 cases of toml-test into std/toml/testdata/toml-test.txt.

    git clone https://github.com/toml-lang/toml-test
    tools/gen_toml_tests.py path/to/toml-test

Each case is a header line, `valid <name> <toml bytes> <json bytes>` or
`invalid <name> <toml bytes>`, then that many bytes of TOML and (for valid
cases) of the expected tagged JSON, each followed by a newline.
"""
import os
import subprocess
import sys

src = os.path.join(sys.argv[1], 'tests')
out = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'std', 'toml', 'testdata', 'toml-test.txt')
commit = subprocess.run(['git', '-C', sys.argv[1], 'rev-parse', 'HEAD'], capture_output=True, text=True).stdout.strip()
names = [l.strip() for l in open(os.path.join(src, 'files-toml-1.0.0')) if l.strip().endswith('.toml')]
os.makedirs(os.path.dirname(out), exist_ok=True)
with open(out, 'wb') as f:
    f.write(b'# toml-test %s, the TOML 1.0.0 cases (tools/gen_toml_tests.py)\n' % commit.encode())
    for n in sorted(names):
        t = open(os.path.join(src, n), 'rb').read()
        kind = n.split('/')[0]
        if kind == 'valid':
            j = open(os.path.join(src, n[:-5] + '.json'), 'rb').read()
            f.write(b'valid %s %d %d\n' % (n.encode(), len(t), len(j)) + t + b'\n' + j + b'\n')
        else:
            f.write(b'invalid %s %d\n' % (n.encode(), len(t)) + t + b'\n')
print(out, len(names), 'cases')
