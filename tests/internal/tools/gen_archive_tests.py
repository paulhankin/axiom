#!/usr/bin/env python3
"""Archives made by Python's zipfile and tarfile, for std/archive/zip and std/archive/tar.

    tests/internal/tools/gen_archive_tests.py

Writes std/archive/zip/testdata/*.zip and std/archive/tar/testdata/*.tar.gz, and for each
a listing `<archive>.txt` with a line per entry: `<kind> <size> <mode octal>
<mtime> <crc32 of the contents, hex> <name>` (kind: f file, d directory,
l symlink with ` -> target` added).
"""
import gzip
import io
import os
import tarfile
import time
import zipfile
import zlib

root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', 'std', 'archive')
MTIME = 1700000000  # 2023-11-14T22:13:20Z (even seconds, for DOS times)

files = [
    ('hello.txt', b'Hello, world!\n'),
    ('dir/nested/deep.txt', b'deep\n' * 1000),
    ('café/ünicode ☃.txt', 'snowman ☃\n'.encode()),
    ('empty', b''),
    ('binary.bin', bytes(range(256)) * 40),
    ('a' * 120 + '/' + 'b' * 120 + '.txt', b'long name\n'),
]


def crc(b):
    return '%08x' % (zlib.crc32(b) & 0xffffffff)


# --- zip
zdir = os.path.join(root, 'zip', 'testdata')
os.makedirs(zdir, exist_ok=True)
for name, method, z64 in [('stored', zipfile.ZIP_STORED, False), ('deflated', zipfile.ZIP_DEFLATED, False),
                          ('zip64', zipfile.ZIP_DEFLATED, True)]:
    buf = io.BytesIO()
    lines = []
    with zipfile.ZipFile(buf, 'w', compression=method) as z:
        z.comment = b'made by tests/internal/tools/gen_archive_tests.py'
        di = zipfile.ZipInfo('dir/', date_time=time.gmtime(MTIME)[:6])
        di.external_attr = (0o40755 << 16) | 0x10
        z.writestr(di, b'')
        lines.append('d 0 755 %d %s dir/' % (MTIME, crc(b'')))
        for fname, data in files:
            zi = zipfile.ZipInfo(fname, date_time=time.gmtime(MTIME)[:6])
            zi.compress_type = method
            zi.external_attr = (0o100644 << 16)
            with z.open(zi, 'w', force_zip64=z64) as f:
                f.write(data)
            lines.append('f %d 644 %d %s %s' % (len(data), MTIME, crc(data), fname))
    open(os.path.join(zdir, name + '.zip'), 'wb').write(buf.getvalue())
    open(os.path.join(zdir, name + '.txt'), 'w').write('\n'.join(lines) + '\n')

# --- tar
tdir = os.path.join(root, 'tar', 'testdata')
os.makedirs(tdir, exist_ok=True)
for name, fmt in [('ustar', tarfile.USTAR_FORMAT), ('gnu', tarfile.GNU_FORMAT), ('pax', tarfile.PAX_FORMAT)]:
    buf = io.BytesIO()
    lines = []
    with tarfile.open(fileobj=buf, mode='w', format=fmt) as t:
        d = tarfile.TarInfo('dir')
        d.type = tarfile.DIRTYPE
        d.mode = 0o755
        d.mtime = MTIME
        t.addfile(d)
        lines.append('d 0 755 %d %s dir' % (MTIME, crc(b'')))
        for fname, data in files:
            if fmt == tarfile.USTAR_FORMAT and (len(fname.encode()) > 100 and '/' not in fname[-100:]):
                continue
            if fmt == tarfile.USTAR_FORMAT and len(fname) > 200:
                continue
            ti = tarfile.TarInfo(fname)
            ti.size = len(data)
            ti.mode = 0o644
            ti.mtime = MTIME
            ti.uname = 'user'
            ti.gname = 'group'
            t.addfile(ti, io.BytesIO(data))
            lines.append('f %d 644 %d %s %s' % (len(data), MTIME, crc(data), fname))
        s = tarfile.TarInfo('link-to-hello')
        s.type = tarfile.SYMTYPE
        s.linkname = 'hello.txt'
        s.mode = 0o777
        s.mtime = MTIME
        t.addfile(s)
        lines.append('l 0 777 %d %s link-to-hello -> hello.txt' % (MTIME, crc(b'')))
    raw = buf.getvalue()
    open(os.path.join(tdir, name + '.tar.gz'), 'wb').write(gzip.compress(raw, mtime=0))
    open(os.path.join(tdir, name + '.txt'), 'w').write('\n'.join(lines) + '\n')
print('ok')
