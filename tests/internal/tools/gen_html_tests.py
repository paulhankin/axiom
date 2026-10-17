#!/usr/bin/env python3
"""Bundles html5lib-tests into std/text/html/testdata.

    git clone https://github.com/html5lib/html5lib-tests
    git -C html5lib-tests checkout 9329e64   # the last commit with tree-construction
    tests/internal/tools/gen_html_tests.py path/to/html5lib-tests

tree.dat is every tree-construction/*.dat file (except scripted/, which
needs a script engine) run together. tokenizer.txt holds the tokenizer
tests: a header line `case <state> <last start tag or -> <input bytes>
<expected bytes>`, the input, a newline, the expected tokens (one per line)
and a newline. A token line is D|name|public|system|correct (a field is
"-" when missing, else "+" and the text), S|name|self-closing|attr|value...
(attributes sorted by name), E|name, C|comment or T|characters (runs
merged); bytes below 0x20 and "\\" and "|" are written as \\xHH. Tests
whose strings hold lone surrogates cannot be UTF-8 and are left out.
"""
import json
import os
import re
import subprocess
import sys

src = sys.argv[1]
out_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', 'std', 'text', 'html', 'testdata')
commit = subprocess.run(['git', '-C', src, 'rev-parse', 'HEAD'], capture_output=True, text=True).stdout.strip()
os.makedirs(out_dir, exist_ok=True)

tree_dir = os.path.join(src, 'tree-construction')
n = 0
with open(os.path.join(out_dir, 'tree.dat'), 'wb') as f:
    f.write(b'# html5lib-tests %s tree-construction (tests/internal/tools/gen_html_tests.py)\n' % commit.encode())
    for name in sorted(os.listdir(tree_dir)):
        if not name.endswith('.dat'):
            continue
        data = open(os.path.join(tree_dir, name), 'rb').read()
        n += data.count(b'#data\n')
        f.write(b'\n' + data.rstrip(b'\n') + b'\n')
print('tree.dat:', n, 'cases')


def unescape(s):
    return re.sub(r'\\u([0-9A-Fa-f]{4})', lambda m: chr(int(m.group(1), 16)), s)


def deep(x):
    if isinstance(x, str):
        return unescape(x)
    if isinstance(x, list):
        return [deep(y) for y in x]
    if isinstance(x, dict):
        return {unescape(k): deep(v) for k, v in x.items()}
    return x


def esc(s):
    b = s.encode('utf-8')
    out = bytearray()
    for c in b:
        if c < 0x20 or c in (0x5c, 0x7c):
            out += b'\\x%02x' % c
        else:
            out.append(c)
    return bytes(out)


def opt(s):
    return b'-' if s is None else b'+' + esc(s)


STATES = {'Data state': b'data', 'PLAINTEXT state': b'plaintext', 'RCDATA state': b'rcdata',
          'RAWTEXT state': b'rawtext', 'Script data state': b'script', 'CDATA section state': b'cdata'}

cases = []
for name in sorted(os.listdir(os.path.join(src, 'tokenizer'))):
    if not name.endswith('.test'):
        continue
    doc = json.load(open(os.path.join(src, 'tokenizer', name)))
    for t in doc.get('tests', []):
        inp = t['input']
        outp = t['output']
        if t.get('doubleEscaped'):
            inp = unescape(inp)
            outp = deep(outp)
        try:
            inb = inp.encode('utf-8')
            lines = []
            for tok in outp:
                k = tok[0]
                if k == 'Character':
                    if lines and lines[-1].startswith(b'T|'):
                        lines[-1] += esc(tok[1])
                    else:
                        lines.append(b'T|' + esc(tok[1]))
                elif k == 'Comment':
                    lines.append(b'C|' + esc(tok[1]))
                elif k == 'EndTag':
                    lines.append(b'E|' + esc(tok[1]))
                elif k == 'StartTag':
                    parts = [b'S', esc(tok[1]), b'1' if len(tok) > 3 and tok[3] else b'0']
                    for an in sorted(tok[2]):
                        parts += [esc(an), esc(tok[2][an])]
                    lines.append(b'|'.join(parts))
                elif k == 'DOCTYPE':
                    lines.append(b'|'.join([b'D', opt(tok[1]), opt(tok[2]), opt(tok[3]), b'1' if tok[4] else b'0']))
                else:
                    raise ValueError(k)
        except UnicodeEncodeError:
            continue  # lone surrogates
        exp = b'\n'.join(lines)
        last = t.get('lastStartTag')
        for st in t.get('initialStates', ['Data state']):
            cases.append((STATES[st], (last or '-').encode(), inb, exp))

with open(os.path.join(out_dir, 'tokenizer.txt'), 'wb') as f:
    f.write(b'# html5lib-tests %s tokenizer (tests/internal/tools/gen_html_tests.py)\n' % commit.encode())
    for st, last, inb, exp in cases:
        f.write(b'case %s %s %d %d\n' % (st, last, len(inb), len(exp)))
        f.write(inb + b'\n' + exp + b'\n')
print('tokenizer.txt:', len(cases), 'cases')
