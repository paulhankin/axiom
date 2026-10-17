#!/usr/bin/env python3
"""Test files for std/compress/snappy, made by the reference library (through
python-snappy, which wraps Google's snappy for the raw format and the Rust snap
crate for the framing format) and by hand from the format descriptions.

    pip install python-snappy
    tests/internal/tools/gen_snappy_tests.py

std/compress/snappy/testdata/ gets NAME.raw (a raw block) or NAME.sz (the
framing format) with NAME.out, what it decompresses to; the library's decoders
agree with every pair. corpus-*.raw and corpus-*.sz decompress to
std/compress/testdata/corpus.bin (made by tests/internal/tools/gen_compress_tests.py) and
have no .out file.
"""
import os

import snappy

here = os.path.dirname(os.path.abspath(__file__))
out_dir = os.path.join(here, '..', '..', '..', 'std', 'compress', 'snappy', 'testdata')
corpus = open(os.path.join(here, '..', '..', '..', 'std', 'compress', 'testdata', 'corpus.bin'), 'rb').read()
os.makedirs(out_dir, exist_ok=True)


def crc32c(data):
    c = 0xffffffff
    for b in data:
        c ^= b
        for _ in range(8):
            c = (c >> 1) ^ (0x82f63b78 if c & 1 else 0)
    return c ^ 0xffffffff


def masked(data):
    c = crc32c(data)
    return (((c >> 15) | (c << 17)) + 0xa282ead8) & 0xffffffff


def varint(n):
    out = bytearray()
    while n >= 0x80:
        out.append((n & 0x7f) | 0x80)
        n >>= 7
    out.append(n)
    return bytes(out)


def chunk(kind, body):
    return bytes([kind]) + len(body).to_bytes(3, 'little') + body


IDENT = chunk(0xff, b'sNaPpY')


def data_chunk(data, raw=None):
    """A compressed chunk (of raw, or of the library's raw encoding), or stored."""
    if raw is False:
        return chunk(1, masked(data).to_bytes(4, 'little') + data)
    return chunk(0, masked(data).to_bytes(4, 'little') + (raw or snappy.compress(data)))


def write(name, data, expect=None):
    open(os.path.join(out_dir, name), 'wb').write(data)
    if expect is not None:
        stem = name.rsplit('.', 1)[0]
        open(os.path.join(out_dir, stem + '.out'), 'wb').write(expect)
    want = corpus if expect is None else expect
    if name.endswith('.raw'):
        assert snappy.uncompress(data) == want, name
    else:
        assert snappy.StreamDecompressor().decompress(data) == want, name


# the library's own output
write('corpus-lib.raw', snappy.compress(corpus))
write('corpus-lib.sz', snappy.StreamCompressor().compress(corpus))
write('empty.raw', snappy.compress(b''), b'')
short = b'Hello, Hello, Hello, world!\n'
write('short.raw', snappy.compress(short), short)
write('short.sz', snappy.StreamCompressor().compress(short), short)

# raw blocks by hand, one element kind at a time (format_description.txt)
lit = bytes(range(256)) * 2
write('lit60.raw', varint(70) + bytes([60 << 2, 69]) + lit[:70], lit[:70])            # 1-byte length
write('lit61.raw', varint(300) + bytes([61 << 2]) + (299).to_bytes(2, 'little') + lit[:300], lit[:300])
write('lit62.raw', varint(300) + bytes([62 << 2]) + (299).to_bytes(3, 'little') + lit[:300], lit[:300])
write('lit63.raw', varint(300) + bytes([63 << 2]) + (299).to_bytes(4, 'little') + lit[:300], lit[:300])
# copy with a 1-byte offset: length 4-11, offset < 2048, overlapping (a run)
write('copy1.raw', varint(15) + bytes([2 << 2]) + b'abc' + bytes([1 | (7 << 2), 3, 0 << 2]) + b'!',
      b'abc' + b'abcabcabcab' + b'!')
# copy with a 2-byte offset of 1 (a run of 64), then one with a far offset
x = bytes((i * 7) & 255 for i in range(3000))
write('copy2.raw', varint(3000 + 64 + 20) + bytes([61 << 2]) + (2999).to_bytes(2, 'little') + x
      + bytes([2 | (63 << 2)]) + (1).to_bytes(2, 'little') + bytes([2 | (19 << 2)]) + (3064).to_bytes(2, 'little'),
      x + bytes([x[-1]]) * 64 + x[:20])
# copy with a 4-byte offset, overlapping
write('copy4.raw', varint(2 + 30) + bytes([1 << 2]) + b'xy' + bytes([3 | (29 << 2)]) + (2).to_bytes(4, 'little'),
      b'xy' * 16)

# framing by hand (framing_format.txt): padding, a reserved skippable chunk, a
# stored chunk, a compressed chunk, then a second stream (concatenation)
a, b, c = b'first chunk, stored. ' * 3, corpus[:20000], corpus[20000:24000]
framed = (IDENT + chunk(0xfe, bytes(10)) + chunk(0x80, b'skip me') + data_chunk(a, False) + data_chunk(b)
          + IDENT + data_chunk(c) + chunk(0xfd, b''))
write('mixed.sz', framed, a + b + c)
write('ident-only.sz', IDENT, b'')
print('wrote', len(os.listdir(out_dir)), 'files to', os.path.normpath(out_dir))
