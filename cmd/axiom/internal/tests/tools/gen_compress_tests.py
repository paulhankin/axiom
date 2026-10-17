#!/usr/bin/env python3
"""Compressed test files for std/compress/lz4, bzip2, xz, zstd and brotli,
made by the reference libraries.

    pip install lz4 zstandard brotli
    cmd/axiom/internal/tests/tools/gen_compress_tests.py

std/compress/testdata/corpus.bin is the input (text, random bytes, runs and UTF-8;
it is kept, and made only if missing). Each std/compress/<format>/testdata/ gets
corpus-<variant>.<ext> files that decompress to it, empty.<ext> for empty
input and short.<ext> for SHORT below.
"""
import bz2
import lzma
import os
import random

import brotli
import lz4.frame
import zstandard

std = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', '..', '..', 'std', 'compress')
SHORT = b'Hello, Hello, Hello, world!\n'

cpath = os.path.join(std, 'testdata', 'corpus.bin')
if not os.path.exists(cpath):
    os.makedirs(os.path.dirname(cpath), exist_ok=True)
    rnd = random.Random(1)
    readme = open(os.path.join(std, '..', 'README.md'), 'rb').read()[:30000]
    parts = [readme, bytes(rnd.getrandbits(8) for _ in range(8000)), b'abcabcabd' * 1200, bytes(4000),
             ('été ☃ \U0001f600 中文 ' * 100).encode(), readme[:5000]]
    open(cpath, 'wb').write(b''.join(parts))
corpus = open(cpath, 'rb').read()


def write(fmt, name, data):
    d = os.path.join(std, fmt, 'testdata')
    os.makedirs(d, exist_ok=True)
    open(os.path.join(d, name), 'wb').write(data)


# lz4 frames
F = lz4.frame
for name, kw in [('default', {}), ('hc', {'compression_level': 12}),
                 ('b64k-linked', {'block_size': F.BLOCKSIZE_MAX64KB, 'block_linked': True}),
                 ('checksums', {'block_checksum': True, 'content_checksum': True, 'store_size': True}),
                 ('b256k-nosize', {'block_size': F.BLOCKSIZE_MAX256KB, 'store_size': False,
                                   'content_checksum': False})]:
    write('lz4', 'corpus-%s.lz4' % name, F.compress(corpus, **kw))
write('lz4', 'empty.lz4', F.compress(b''))
write('lz4', 'short.lz4', F.compress(SHORT))

# bzip2
for lvl in (1, 9):
    write('bzip2', 'corpus-%d.bz2' % lvl, bz2.compress(corpus, lvl))
write('bzip2', 'corpus-multistream.bz2', bz2.compress(corpus[:20000]) + bz2.compress(corpus[20000:]))
write('bzip2', 'empty.bz2', bz2.compress(b''))
write('bzip2', 'short.bz2', bz2.compress(SHORT))

# xz and lzma
for name, kw in [('0', {'preset': 0}), ('6', {'preset': 6}), ('9e', {'preset': 9 | lzma.PRESET_EXTREME}),
                 ('none', {'check': lzma.CHECK_NONE}), ('crc32', {'check': lzma.CHECK_CRC32}),
                 ('sha256', {'check': lzma.CHECK_SHA256}),
                 ('x86', {'filters': [{'id': lzma.FILTER_X86}, {'id': lzma.FILTER_LZMA2, 'preset': 6}]}),
                 ('delta', {'filters': [{'id': lzma.FILTER_DELTA, 'dist': 4}, {'id': lzma.FILTER_LZMA2}]}),
                 ('lc0lp2pb0', {'filters': [{'id': lzma.FILTER_LZMA2, 'lc': 0, 'lp': 2, 'pb': 0}]})]:
    write('xz', 'corpus-%s.xz' % name, lzma.compress(corpus, format=lzma.FORMAT_XZ, **kw))
write('xz', 'corpus-multistream.xz', lzma.compress(corpus[:30000]) + lzma.compress(corpus[30000:]))
write('xz', 'corpus-alone.lzma', lzma.compress(corpus, format=lzma.FORMAT_ALONE))
write('xz', 'empty.xz', lzma.compress(b''))
write('xz', 'short.xz', lzma.compress(SHORT))

# zstd
for lvl in (1, 3, 19, 22):
    write('zstd', 'corpus-%d.zst' % lvl, zstandard.ZstdCompressor(level=lvl).compress(corpus))
write('zstd', 'corpus-checksum.zst', zstandard.ZstdCompressor(level=3, write_checksum=True).compress(corpus))
cobj = zstandard.ZstdCompressor(level=5, write_content_size=False).compressobj()
write('zstd', 'corpus-streamed.zst', cobj.compress(corpus[:40000]) + cobj.compress(corpus[40000:]) + cobj.flush())
skip = b'\x50\x2a\x4d\x18' + (5).to_bytes(4, 'little') + b'skip!'
z = zstandard.ZstdCompressor(level=3)
write('zstd', 'corpus-frames.zst', z.compress(corpus[:30000]) + skip + z.compress(corpus[30000:]))
write('zstd', 'empty.zst', zstandard.ZstdCompressor().compress(b''))
write('zstd', 'short.zst', zstandard.ZstdCompressor().compress(SHORT))

# brotli
for name, kw in [('q0', {'quality': 0}), ('q5', {'quality': 5}), ('q11', {'quality': 11}),
                 ('q11-w24', {'quality': 11, 'lgwin': 24}), ('q9-w10', {'quality': 9, 'lgwin': 10}),
                 ('text', {'quality': 7, 'mode': brotli.MODE_TEXT}), ('font', {'quality': 7, 'mode': brotli.MODE_FONT})]:
    write('brotli', 'corpus-%s.br' % name, brotli.compress(corpus, **kw))
write('brotli', 'empty.br', brotli.compress(b''))
write('brotli', 'short.br', brotli.compress(SHORT))
write('brotli', 'short-q11.br', brotli.compress(SHORT, quality=11))
print('ok', len(corpus))
