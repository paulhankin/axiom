#!/usr/bin/env python3
"""Times reference implementations on the inputs tests/bench/std.ax saves.

    axiom build --target=native -O2 tests/bench/std.ax -o build/bench/std
    build/bench/std            # writes build/bench/*, prints Axiom's numbers
    tools/bench_std.py         # the same work in zlib, hashlib, json, re, ...

Each reference is optional: a missing module is skipped. The C libraries
behind them (zlib, OpenSSL, libjpeg, cmark-gfm) are the comparison; the
Python overhead is small for these bulk operations.
"""
import hashlib
import json
import os
import re
import sys
import time
import zlib

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
DIR = os.path.join(ROOT, 'build', 'bench')


def report(name, nbytes, t0):
    s = time.perf_counter() - t0
    print('%-28s %10.1f MB/s  (%.3f s)' % (name, nbytes / 1e6 / max(s, 1e-9), s))


def read(name):
    with open(os.path.join(DIR, name), 'rb') as f:
        return f.read()


def main():
    want = sys.argv[1] if len(sys.argv) > 1 else ''
    text = read('text.txt')

    def on(name):
        return want in name

    if on('deflate'):
        t0 = time.perf_counter()
        z = zlib.compress(text, 6)
        report('deflate compress -6', len(text), t0)
        print('%-28s %10d bytes' % ('  (compressed size)', len(z)))
        t0 = time.perf_counter()
        for _ in range(4):
            zlib.decompress(z)
        report('deflate decompress', len(text) * 4, t0)
    if on('sha') or on('md5'):
        for name in ['sha256', 'sha512', 'md5']:
            t0 = time.perf_counter()
            for _ in range(4):
                hashlib.new(name, text).digest()
            report(name, len(text) * 4, t0)
    if on('aes') or on('chacha'):
        try:
            from cryptography.hazmat.primitives.ciphers.aead import AESGCM, ChaCha20Poly1305
            for name, c in [('aes-128-gcm seal', AESGCM(bytes(16))), ('chacha20-poly1305 seal', ChaCha20Poly1305(bytes(32)))]:
                t0 = time.perf_counter()
                for _ in range(4):
                    c.encrypt(bytes(12), text, b'')
                report(name, len(text) * 4, t0)
        except ImportError:
            print('(no cryptography module: skipping AEADs)')
    if on('base64'):
        import base64
        t0 = time.perf_counter()
        for _ in range(8):
            enc = base64.b64encode(text)
        report('base64 encode', len(text) * 8, t0)
        t0 = time.perf_counter()
        for _ in range(8):
            base64.b64decode(enc)
        report('base64 decode', len(enc) * 8, t0)
    if on('json'):
        data = read('data.json')
        t0 = time.perf_counter()
        for _ in range(4):
            v = json.loads(data)
        report('json parse', len(data) * 4, t0)
        t0 = time.perf_counter()
        for _ in range(4):
            json.dumps(v)
        report('json stringify', len(data) * 4, t0)
    if on('regex'):
        t0 = time.perf_counter()
        len(re.findall(rb'[a-z]+@[a-z]+\.com', text))
        report('regex find_all (emails)', len(text), t0)
        t0 = time.perf_counter()
        len(re.findall(rb'(\w+) (\w+)ing', text))
        report('regex find_all (words)', len(text), t0)
        t0 = time.perf_counter()
        for _ in range(16):
            text.find(b'zzzzqqqq')
        report('str.index_of (miss)', len(text) * 16, t0)
    if on('markdown'):
        try:
            import cmarkgfm
            md = read('doc.md').decode('utf-8')
            t0 = time.perf_counter()
            for _ in range(4):
                cmarkgfm.github_flavored_markdown_to_html(md)
            report('markdown to_html (gfm)', len(md.encode()) * 4, t0)
        except ImportError:
            print('(no cmarkgfm: skipping markdown)')
    if on('png') or on('jpeg') or on('resize'):
        try:
            import io
            from PIL import Image
            im = Image.open(os.path.join(DIR, 'image.png'))
            im.load()
            raw = len(im.tobytes())
            t0 = time.perf_counter()
            out = io.BytesIO()
            im.save(out, 'PNG', compress_level=6)
            report('png encode', raw, t0)
            print('%-28s %10d bytes' % ('  (encoded size)', out.tell()))
            data = read('image.png')
            t0 = time.perf_counter()
            for _ in range(4):
                Image.open(io.BytesIO(data)).load()
            report('png decode', raw * 4, t0)
            rgb = im.convert('RGB')
            t0 = time.perf_counter()
            out = io.BytesIO()
            rgb.save(out, 'JPEG', quality=90)
            report('jpeg encode q90', raw, t0)
            data = read('image.jpg')
            t0 = time.perf_counter()
            for _ in range(4):
                Image.open(io.BytesIO(data)).load()
            report('jpeg decode', raw * 4, t0)
            small = im.resize((im.width // 3, im.height // 3), Image.LANCZOS)
            t0 = time.perf_counter()
            small.resize((im.width, im.height), Image.LANCZOS)
            report('image resize (upscale)', raw, t0)
        except ImportError:
            print('(no Pillow: skipping images)')
    if on('bigint'):
        m = (1 << 2047) | 0x2545f4914f6cdd1d
        x = pow(3, 1300, m)
        t0 = time.perf_counter()
        for _ in range(10):
            pow(x, m - 2, m)
        print('%-28s %10.2f ms each' % ('bigint mod_pow 2048', (time.perf_counter() - t0) * 100))
    if on('pdf'):
        try:
            import pypdf
            data = read('text.pdf')
            t0 = time.perf_counter()
            r = pypdf.PdfReader(os.path.join(DIR, 'text.pdf'))
            n = sum(len(p.extract_text()) for p in r.pages)
            report('pdf read text (pypdf)', n, t0)
        except ImportError:
            print('(no pypdf: skipping pdf)')


if __name__ == '__main__':
    main()
