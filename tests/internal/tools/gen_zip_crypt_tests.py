#!/usr/bin/env python3
"""Generates the encrypted archives in std/archive/zip/testdata for
std/archive/zip's tests (password "secret"):

  zipcrypto.zip         traditional PKWARE encryption, by Info-ZIP's zip -P
  zipcrypto_stream.zip  the same, written to a pipe (data descriptors, so the
                        check byte comes from the modification time)
  aes128.zip, aes192.zip, aes256.zip
                        WinZip AES (AE-2: no CRC), by pyzipper
  aes256_ae1.zip        aes256.zip turned into AE-1 (the version is 1 and the
                        real CRC-32s are recorded; the MAC covers only the
                        encrypted data, so it still holds) (with random salts and the time of generation)

Each holds hello.txt ("Hello, world!\\n", deflated), data.bin (20000
pseudo-random bytes, stored or deflated) and empty.txt.
"""
import os
import struct
import subprocess
import zlib
import tempfile

import pyzipper

root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', 'std', 'archive', 'zip', 'testdata')
PASSWORD = b'secret'


def files():
    x = 12345
    data = bytearray()
    for _ in range(20000):
        x = (x * 1103515245 + 12345) & 0x7fffffff
        data.append((x >> 16) & 0xff if len(data) % 3 else 0x41)
    return [('hello.txt', b'Hello, world!\n'), ('data.bin', bytes(data)), ('empty.txt', b'')]


def main():
    os.makedirs(root, exist_ok=True)
    with tempfile.TemporaryDirectory() as d:
        for name, body in files():
            with open(os.path.join(d, name), 'wb') as f:
                f.write(body)
            os.utime(os.path.join(d, name), (1700000000, 1700000000))
        names = [n for n, _ in files()]
        out = os.path.join(root, 'zipcrypto.zip')
        if os.path.exists(out):
            os.remove(out)
        subprocess.run(['zip', '-q', '-X', '-P', 'secret', out] + names, cwd=d, check=True)
        with open(os.path.join(root, 'zipcrypto_stream.zip'), 'wb') as f:
            p = subprocess.run(['zip', '-q', '-X', '-P', 'secret', '-'] + names, cwd=d, check=True,
                               stdout=subprocess.PIPE)
            f.write(p.stdout)
    for bits in (128, 192, 256):
        out = os.path.join(root, 'aes%d.zip' % bits)
        with pyzipper.AESZipFile(out, 'w', compression=pyzipper.ZIP_DEFLATED, encryption=pyzipper.WZ_AES) as z:
            z.setpassword(PASSWORD)
            z.setencryption(pyzipper.WZ_AES, nbits=bits)
            for name, body in files():
                z.writestr(name, body,
                           compress_type=pyzipper.ZIP_STORED if name == 'data.bin' and bits == 192 else pyzipper.ZIP_DEFLATED)
    to_ae1(os.path.join(root, 'aes256.zip'), os.path.join(root, 'aes256_ae1.zip'))


def to_ae1(src, dst):
    crcs = {n: zlib.crc32(b) for n, b in files()}
    d = bytearray(open(src, 'rb').read())

    def patch(at, name_at, nlen, xlen, x_at, crc_at):
        name = bytes(d[name_at:name_at + nlen]).decode()
        struct.pack_into('<I', d, crc_at, crcs[name])
        j = x_at
        while j + 4 <= x_at + xlen:
            hid, ln = struct.unpack_from('<HH', d, j)
            if hid == 0x9901:
                struct.pack_into('<H', d, j + 4, 1)
            j += 4 + ln

    i = 0
    while i + 4 <= len(d):
        sig = struct.unpack_from('<I', d, i)[0]
        if sig == 0x04034b50:
            nlen, xlen = struct.unpack_from('<HH', d, i + 26)
            patch(i, i + 30, nlen, xlen, i + 30 + nlen, i + 14)
            i += 30 + nlen + xlen + struct.unpack_from('<I', d, i + 18)[0]
        elif sig == 0x02014b50:
            nlen, xlen, klen = struct.unpack_from('<HHH', d, i + 28)
            patch(i, i + 46, nlen, xlen, i + 46 + nlen, i + 16)
            i += 46 + nlen + xlen + klen
        else:
            break
    open(dst, 'wb').write(d)


if __name__ == '__main__':
    main()
