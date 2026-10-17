#!/usr/bin/env python3
"""Generates std/text/wrap/testdata/cases.json: random ASCII texts wrapped,
dedented and indented by Python's textwrap.

Each case is an object: {"op": "wrap", "text", "width", "initial_indent",
"subsequent_indent", "drop_whitespace", "break_long_words",
"break_on_hyphens", "tabsize" (0: expand_tabs=False), "want": [lines]},
{"op": "dedent", "text", "want"} or {"op": "indent", "text", "prefix", "want"}.

The texts avoid the few places where textwrap's word pattern and UAX #14
differ (hyphens after a single letter or in runs, slashes, quotes).
"""
import json
import os
import random
import textwrap

rng = random.Random(31337)
LETTERS = 'abcdefghijklmnopqrstuvwxyz'


def word():
    k = rng.random()
    if k < 0.08:
        return rng.choice(LETTERS[:3]) * rng.randint(12, 40)  # long
    w = ''.join(rng.choice(LETTERS) for _ in range(rng.randint(1, 9)))
    if k < 0.2:
        w = ''.join(rng.choice(LETTERS) for _ in range(rng.randint(2, 6))) + '-' + \
            ''.join(rng.choice(LETTERS) for _ in range(rng.randint(2, 7)))
    elif k < 0.25:
        w = str(rng.randint(0, 99999))
    elif k < 0.3:
        w = w.capitalize()
    if rng.random() < 0.15:
        w += rng.choice('.,;:!?')
    return w


def space():
    k = rng.random()
    if k < 0.75:
        return ' '
    if k < 0.85:
        return ' ' * rng.randint(2, 4)
    if k < 0.92:
        return '\n'
    return rng.choice(['\t', ' \t', '\t\t', '  \n  '])


def text():
    n = rng.randint(0, 30)
    s = ''
    if rng.random() < 0.15:
        s += rng.choice(['  ', '\t', ' '])
    for i in range(n):
        s += word()
        if i + 1 < n:
            s += space()
    if rng.random() < 0.15:
        s += rng.choice([' ', '  ', '\n'])
    return s


cases = []
for i in range(600):
    width = rng.choice([1, 2, 3, 5, 8, 10, 12, 15, 20, 25, 30, 40, 60, 80])
    ii = rng.choice(['', '', '', '* ', '    ', '>>> '])
    si = rng.choice(['', '', '', '  ', '    ', '. '])
    drop = rng.random() < 0.85
    blw = rng.random() < 0.85
    boh = rng.random() < 0.8
    expand = rng.random() < 0.85
    tabsize = rng.choice([8, 8, 4, 1]) if expand else 0
    t = text()
    if width <= max(len(ii), len(si)):
        # an indent as wide as the line can make textwrap (3.11) loop forever
        width = max(len(ii), len(si)) + rng.randint(1, 10)
    w = textwrap.TextWrapper(width=width, initial_indent=ii, subsequent_indent=si, drop_whitespace=drop,
                             break_long_words=blw, break_on_hyphens=boh, expand_tabs=expand,
                             tabsize=tabsize if expand else 8)
    cases.append({'op': 'wrap', 'text': t, 'width': width, 'initial_indent': ii, 'subsequent_indent': si,
                  'drop_whitespace': drop, 'break_long_words': blw, 'break_on_hyphens': boh,
                  'tabsize': tabsize, 'want': w.wrap(t)})


def block():
    lines = []
    base = rng.choice(['', '  ', '    ', '\t', ' \t'])
    for _ in range(rng.randint(0, 6)):
        k = rng.random()
        if k < 0.15:
            lines.append('')
        elif k < 0.25:
            lines.append(rng.choice([' ', '  ', '\t', ' \t ']))
        else:
            extra = rng.choice(['', '', '  ', '\t', ' '])
            if rng.random() < 0.15:
                lines.append(rng.choice(['', ' ', '\t']) + word())
            else:
                lines.append(base + extra + ' '.join(word() for _ in range(rng.randint(1, 4))))
    s = '\n'.join(lines)
    if rng.random() < 0.5:
        s += '\n'
    return s


for i in range(150):
    t = block()
    cases.append({'op': 'dedent', 'text': t, 'want': textwrap.dedent(t)})
for i in range(100):
    t = block()
    if rng.random() < 0.3:
        t = t.replace('\n', rng.choice(['\r\n', '\r', '\x0b', '\x0c']), rng.randint(1, 3))
    prefix = rng.choice(['> ', '    ', '# ', '\t'])
    cases.append({'op': 'indent', 'text': t, 'prefix': prefix, 'want': textwrap.indent(t, prefix)})

here = os.path.dirname(os.path.abspath(__file__))
out = os.path.join(here, '..', '..', '..', 'std', 'text', 'wrap', 'testdata', 'cases.json')
with open(out, 'w') as f:
    f.write('[\n')
    f.write(',\n'.join(json.dumps(c) for c in cases))
    f.write('\n]\n')
