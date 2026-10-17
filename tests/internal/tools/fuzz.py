#!/usr/bin/env python3
"""Mutation fuzzing of the standard library's parsers.

    tests/internal/tools/fuzz.py [--runs N] [--timeout S] [--jobs J] [--seed K] [target ...]

Builds tests/fuzz/fuzz.ax natively, makes seed inputs (with std's own
encoders, plus sounds from ffmpeg/sox, system fonts and certificates when
they are around), then runs mutated seeds through each target. Bad input is
expected to fail with an error; a trap, a signal or a hang is a bug, and
the input that caused it is saved under build/fuzz/failures/<target>/.
Targets: image audio deflate json regex markdown pdf font x509 text web
structured compress archive svg collate opusenc binary math (all by default).
"""
import argparse
import concurrent.futures
import glob
import os
import random
import shutil
import subprocess
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..'))
OUT = os.path.join(ROOT, 'build', 'fuzz')
TARGETS = ['image', 'audio', 'deflate', 'json', 'regex', 'markdown', 'pdf', 'font', 'x509', 'text', 'web', 'structured',
           'compress', 'archive', 'svg', 'collate', 'opusenc', 'binary', 'math']
TOKENS = {
    'json': [b'{', b'}', b'[', b']', b'"', b'\\u', b'\\ud800', b'1e999', b'-', b'[' * 600, b'null', b','],
    'regex': [b'(', b')', b'*', b'+', b'?', b'{', b'}', b'{1000}', b'{2,', b'[', b']', b'\\', b'|', b'(?P<', b'\x00'],
    'markdown': [b'$', b'$$', b'\\frac{', b'```math\n',
                 b'*', b'_', b'[', b']', b'(', b')', b'<', b'>', b'`', b'\n', b'    ', b'- ', b'> ', b'|', b'~~',
                 b'&', b'#', b'\t', b'!['],
    'pdf': [b'<<', b'>>', b'[', b']', b'(', b')', b'R', b' 0 obj', b'endobj', b'stream\n', b'endstream',
            b'/Length 1', b'99999999', b'-1', b'/Filter /FlateDecode', b'xref', b'trailer'],
    'web': [b';', b'=', b'%', b'%zz', b'/', b'..', b'[', b']', b':', b'@', b'?', b'#', b'*', b',', b'-', b'Expires=',
            b'Max-Age=-1', b'Domain=', b'.', b'\xff'],
    'structured': [b'[', b']', b'{', b'}', b'<', b'>', b'&', b'"', b"'", b'\n', b'  ', b'- ', b': ', b'*', b'!!', b'<!--',
                b'\xff', b'\x9f', b'\xbf', b'\x1b'],
    'collate': [b'\xcc\x81', b'\xe0\xbd\xb1', b'\xe0\xbe\x80', b'\xe4\xb8\x80', b'\xff', b'\xed\xa0\x80', b' ', b'-'],
    'math': [b'{', b'}', b'^', b'_', b'\\frac', b'\\left(', b'\\right)', b'\\middle|', b'\\begin{pmatrix}',
             b'\\end{pmatrix}', b'&', b'\\\\', b'\\sqrt[', b']', b'\\text{', b'\\mathbb{', b'\\not', b'\\over',
             b'\\color{red}', b"'", b'{' * 70, b'\xff'],
    'text': [b'\xff', b'\xc3', b'\xcc\x81', b'\xe2\x80\x8f', b'\xd7\xa9', b'(', b')', b'%', b':', b'-', b'9' * 30],
}


def build(compiler):
    os.makedirs(OUT, exist_ok=True)
    exe = os.path.join(OUT, 'fuzz')
    subprocess.run([compiler, 'build', '--target=native', '-O2', '-o', exe,
                    os.path.join(ROOT, 'tests', 'fuzz', 'fuzz.ax')], check=True)
    return exe


def make_seeds(exe):
    seeds = os.path.join(OUT, 'seeds')
    if os.path.isdir(seeds):
        shutil.rmtree(seeds)
    os.makedirs(seeds)
    subprocess.run([exe, 'seeds', seeds], check=True)
    audio = os.path.join(seeds, 'audio')
    # compressed sounds, if the tools are here
    if shutil.which('ffmpeg'):
        for name, args in [('tone.mp3', ['-b:a', '32k']), ('tone.ogg', ['-c:a', 'libvorbis', '-q:a', '0'])]:
            subprocess.run(['ffmpeg', '-loglevel', 'quiet', '-y', '-f', 'lavfi', '-i',
                            'sine=frequency=440:duration=0.2', '-ac', '1'] + args + [os.path.join(audio, name)])
    fonts = os.path.join(seeds, 'font')
    os.makedirs(fonts, exist_ok=True)
    for pat in ['/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf', '/usr/share/fonts/opentype/*/*.otf']:
        for f in sorted(glob.glob(pat))[:2]:
            if os.path.getsize(f) < 400000:
                shutil.copy(f, fonts)
    arch = os.path.join(seeds, 'archive')
    for f in sorted(glob.glob(os.path.join(ROOT, 'std', 'archive', 'zip', 'testdata', '*.zip')))[:6]:
        if os.path.getsize(f) < 30000:
            shutil.copy(f, arch)
    for f in sorted(glob.glob(os.path.join(ROOT, 'std', 'graphics', 'svg', 'testdata', '*.svg')))[:20]:
        shutil.copy(f, os.path.join(seeds, 'svg'))
    certs = os.path.join(seeds, 'x509')
    os.makedirs(certs, exist_ok=True)
    for f in sorted(glob.glob('/etc/ssl/certs/*.pem'))[:4]:
        shutil.copy(f, certs)
    return seeds


def mutate(data, rng, tokens):
    d = bytearray(data)
    for _ in range(rng.choice([1, 1, 2, 3, 5, 8, 16])):
        r = rng.random()
        p = rng.randrange(len(d) + 1)
        if not d or r < 0.05:
            d[p:p] = bytes(rng.randrange(256) for _ in range(rng.randint(1, 8)))
        elif r < 0.35:
            d[min(p, len(d) - 1)] = rng.randrange(256)
        elif r < 0.45:
            d[min(p, len(d) - 1)] ^= 1 << rng.randrange(8)
        elif r < 0.55:
            # an interesting integer, little or big endian
            v = rng.choice([0, 1, 0x7f, 0x80, 0xff, 0x7fff, 0xffff, 0x7fffffff, 0xffffffff])
            n = rng.choice([1, 2, 4])
            b = (v & ((1 << (8 * n)) - 1)).to_bytes(n, rng.choice(['little', 'big']))
            d[p:p + n] = b
        elif r < 0.7:
            del d[p:p + rng.randint(1, 64)]
        elif r < 0.85:
            q = rng.randrange(len(d))
            d[p:p] = d[q:q + rng.randint(1, 64)]
        elif tokens:
            d[p:p] = rng.choice(tokens)
        else:
            d[p:p + 1] = bytes([rng.randrange(256)])
    return bytes(d)


def run_one(exe, target, data, path, timeout):
    with open(path, 'wb') as f:
        f.write(data)
    try:
        r = subprocess.run([exe, target, path], capture_output=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        return 'hang', b''
    if r.returncode in (0, 1):
        return None, b''
    return 'crash %d' % r.returncode, r.stderr[-2000:]


def fuzz_target(exe, seeds, target, runs, timeout, seed):
    rng = random.Random('%d-%s' % (seed, target))
    files = sorted(glob.glob(os.path.join(seeds, target, '*')))
    corpus = [open(f, 'rb').read() for f in files]
    if not corpus:
        return target, 0, 0, ['no seeds']
    fails = os.path.join(OUT, 'failures', target)
    work = os.path.join(OUT, 'work-%s.bin' % target)
    found = []
    for i in range(runs):
        data = mutate(rng.choice(corpus), rng, TOKENS.get(target))
        kind, err = run_one(exe, target, data, work, timeout)
        if kind:
            os.makedirs(fails, exist_ok=True)
            name = os.path.join(fails, '%s-%d' % (kind.split()[0], i))
            with open(name, 'wb') as f:
                f.write(data)
            msg = err.decode('utf-8', 'replace').strip().split('\n')[0] if err else kind
            found.append('%s: %s' % (os.path.relpath(name, ROOT), msg))
    return target, runs, len(found), found


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--runs', type=int, default=500)
    ap.add_argument('--timeout', type=float, default=10.0)
    ap.add_argument('--jobs', type=int, default=os.cpu_count() or 2)
    ap.add_argument('--seed', type=int, default=1)
    ap.add_argument('--compiler', default=os.environ.get('AXIOM', os.path.join(ROOT, 'cmd', 'axiom', 'bootstrap', 'build', 'axiom')))
    ap.add_argument('targets', nargs='*')
    a = ap.parse_args()
    targets = a.targets or TARGETS
    exe = build(a.compiler)
    seeds = make_seeds(exe)
    total = 0
    with concurrent.futures.ThreadPoolExecutor(max_workers=a.jobs) as ex:
        futs = [ex.submit(fuzz_target, exe, seeds, t, a.runs, a.timeout, a.seed) for t in targets]
        for fu in concurrent.futures.as_completed(futs):
            target, runs, bad, found = fu.result()
            total += bad
            print('%-9s %6d runs, %d failures' % (target, runs, bad))
            for f in found[:20]:
                print('    ' + f)
    sys.exit(1 if total else 0)


if __name__ == '__main__':
    main()
