# cmd/axiom/internal/tests/tools/gen_insn_tests.py - writes cmd/axiom/internal/tests/run/insn.ax: the processor-instruction builtins against a
# model of AES, carry-less multiplication and CRC in Python:
#     python3 cmd/axiom/internal/tests/tools/gen_insn_tests.py > cmd/axiom/internal/tests/run/insn.ax && axiom fmt cmd/axiom/internal/tests/run/insn.ax
import random
random.seed(7)

def xt(b):
    return ((b << 1) ^ 0x1b) & 0xff if b & 0x80 else b << 1

def mul(a, b):
    r = 0
    while b:
        if b & 1:
            r ^= a
        a = xt(a)
        b >>= 1
    return r

def inv(a):
    if a == 0:
        return 0
    for b in range(1, 256):
        if mul(a, b) == 1:
            return b

SBOX = []
for x in range(256):
    b = inv(x)
    s = b
    for k in range(1, 5):
        s ^= ((b << k) | (b >> (8 - k))) & 0xff
    SBOX.append(s ^ 0x63)
ISBOX = [0] * 256
for i, s in enumerate(SBOX):
    ISBOX[s] = i

def sub(s): return [SBOX[x] for x in s]
def isub(s): return [ISBOX[x] for x in s]
def shift(s): return [s[(i + 4 * (i % 4)) % 16] for i in range(16)]
def ishift(s): return [s[(i - 4 * (i % 4)) % 16] for i in range(16)]
def mix(s):
    r = []
    for c in range(4):
        a = s[4 * c:4 * c + 4]
        r += [mul(a[0], 2) ^ mul(a[1], 3) ^ a[2] ^ a[3], a[0] ^ mul(a[1], 2) ^ mul(a[2], 3) ^ a[3],
              a[0] ^ a[1] ^ mul(a[2], 2) ^ mul(a[3], 3), mul(a[0], 3) ^ a[1] ^ a[2] ^ mul(a[3], 2)]
    return r
def imix(s):
    r = []
    for c in range(4):
        a = s[4 * c:4 * c + 4]
        r += [mul(a[0], 14) ^ mul(a[1], 11) ^ mul(a[2], 13) ^ mul(a[3], 9),
              mul(a[0], 9) ^ mul(a[1], 14) ^ mul(a[2], 11) ^ mul(a[3], 13),
              mul(a[0], 13) ^ mul(a[1], 9) ^ mul(a[2], 14) ^ mul(a[3], 11),
              mul(a[0], 11) ^ mul(a[1], 13) ^ mul(a[2], 9) ^ mul(a[3], 14)]
    return r
def xor(a, b): return [x ^ y for x, y in zip(a, b)]

def aesenc(s, k): return xor(mix(shift(sub(s))), k)
def aesenclast(s, k): return xor(shift(sub(s)), k)
def aesdec(s, k): return xor(imix(ishift(isub(s))), k)
def aesdeclast(s, k): return xor(ishift(isub(s)), k)
def keygen(s, imm):
    d = [int.from_bytes(bytes(s[4 * i:4 * i + 4]), 'little') for i in range(4)]
    def sw(w): return int.from_bytes(bytes(SBOX[b] for b in w.to_bytes(4, 'little')), 'little')
    def rot(w): return ((w >> 8) | (w << 24)) & 0xffffffff
    r = [sw(d[1]), rot(sw(d[1])) ^ imm, sw(d[3]), rot(sw(d[3])) ^ imm]
    return list(b''.join(x.to_bytes(4, 'little') for x in r))
def aese(s, k): return shift(sub(xor(s, k)))
def aesd(s, k): return ishift(isub(xor(s, k)))

def clmul64(x, y):
    r = 0
    for i in range(64):
        if (y >> i) & 1:
            r ^= x << i
    return r

def crc(data, poly):
    c = 0xffffffff
    for b in data:
        c ^= b
        for _ in range(8):
            c = (c >> 1) ^ (poly if c & 1 else 0)
    return c ^ 0xffffffff

def bvec(xs): return '@vec(u8x16, %s)' % ', '.join(str(x) for x in xs)
def qvec(xs): return '@vec(u64x2, %s)' % ', '.join(hex(x) for x in xs)

st = [random.randrange(256) for _ in range(16)]
key = [random.randrange(256) for _ in range(16)]
qa = [random.getrandbits(64) for _ in range(2)]
qb = [random.getrandbits(64) for _ in range(2)]
out = []
P = out.append
P('// The processor-instruction builtins (spec §13.5) against values from a')
P('// model of AES, carry-less multiplication and CRC (written by')
P('// cmd/axiom/internal/tests/tools/gen_insn_tests.py). Each architecture checks its own in a function')
P('// compiled for the extensions, run where the processor has them.')
P('import "../lib/io.ax" as io;')
P('')
P('fn same(a: u8x16, b: u8x16) bool {')
P('    return @all_true(@lanes_eq(a, b));')
P('}')
P('')
P('fn same64(a: u64x2, b: u64x2) bool {')
P('    return @all_true(@lanes_eq(a, b));')
P('}')
P('')
P('fn x86(s: u8x16, k: u8x16) bool @features("aes,pclmul,sse4.2") {')
P('    if @target_arch("x86_64") {')
P('        let mut ok = true;')
for name, fn in [('aesenc', aesenc), ('aesenclast', aesenclast), ('aesdec', aesdec), ('aesdeclast', aesdeclast)]:
    P('        ok = ok and same(@x86_%s(s, k), %s);' % (name, bvec(fn(st, key))))
P('        ok = ok and same(@x86_aesimc(s), %s);' % bvec(imix(st)))
P('        ok = ok and same(@x86_aeskeygenassist(s, 0x1b), %s);' % bvec(keygen(st, 0x1b)))
P('        let a = %s;' % qvec(qa))
P('        let b = %s;' % qvec(qb))
for imm in (0x00, 0x01, 0x10, 0x11):
    p = clmul64(qa[imm & 1], qb[imm >> 4])
    P('        ok = ok and same64(@x86_clmul(a, b, %s), %s);' % (hex(imm), qvec([p & (2**64 - 1), p >> 64])))
P('        let text = "123456789";')
P('        let mut c: u32 = 0xffff_ffff;')
P('        for x in text {')
P('            c = @x86_crc32c(c, x);')
P('        }')
P('        ok = ok and (~c == %s);' % hex(crc(b'123456789', 0x82f63b78)))
P('        // the wider forms, eight bytes at a time')
P('        c = @x86_crc32c(0xffff_ffff, @as(u64, 0x3837_3635_3433_3231));')
P('        c = @x86_crc32c(c, @as(u8, 0x39));')
P('        ok = ok and (~c == %s);' % hex(crc(b'123456789', 0x82f63b78)))
P('        return ok;')
P('    }')
P('    return false;')
P('}')
P('')
P('fn arm(s: u8x16, k: u8x16) bool @features("aes,sha2,sha3,crc") {')
P('    if @target_arch("aarch64") {')
P('        let mut ok = true;')
P('        ok = ok and same(@arm_aese(s, k), %s);' % bvec(aese(st, key)))
P('        ok = ok and same(@arm_aesd(s, k), %s);' % bvec(aesd(st, key)))
P('        ok = ok and same(@arm_aesmc(s), %s);' % bvec(mix(st)))
P('        ok = ok and same(@arm_aesimc(s), %s);' % bvec(imix(st)))
P('        let a = %s;' % qvec(qa))
P('        let b = %s;' % qvec(qb))
p0, p1 = clmul64(qa[0], qb[0]), clmul64(qa[1], qb[1])
P('        ok = ok and same64(@arm_pmull(a, b), %s);' % qvec([p0 & (2**64 - 1), p0 >> 64]))
P('        ok = ok and same64(@arm_pmull2(a, b), %s);' % qvec([p1 & (2**64 - 1), p1 >> 64]))
c3 = [random.getrandbits(64) for _ in range(2)]
P('        let c = %s;' % qvec(c3))
P('        ok = ok and same64(@arm_eor3(a, b, c), %s);' % qvec([qa[i] ^ qb[i] ^ c3[i] for i in range(2)]))
P('        ok = ok and same64(@arm_bcax(a, b, c), %s);' % qvec([qa[i] ^ (qb[i] & ~c3[i] & (2**64 - 1)) for i in range(2)]))
rol = lambda x, k: ((x << k) | (x >> (64 - k))) & (2**64 - 1)
P('        ok = ok and same64(@arm_rax1(a, b), %s);' % qvec([qa[i] ^ rol(qb[i], 1) for i in range(2)]))
P('        ok = ok and same64(@arm_xar(a, b, 13), %s);' % qvec([rol(qa[i] ^ qb[i], 64 - 13) for i in range(2)]))
P('        ok = ok and (@arm_sha1h(0x1234_5678) == %s);' % hex(((0x12345678 << 30) | (0x12345678 >> 2)) & 0xffffffff))
P('        let text = "123456789";')
P('        let mut x: u32 = 0xffff_ffff;')
P('        let mut y: u32 = 0xffff_ffff;')
P('        for ch in text {')
P('            x = @arm_crc32(x, ch);')
P('            y = @arm_crc32c(y, ch);')
P('        }')
P('        ok = ok and (~x == %s) and (~y == %s);' % (hex(crc(b'123456789', 0xedb88320)), hex(crc(b'123456789', 0x82f63b78))))
P('        x = @arm_crc32(0xffff_ffff, @as(u64, 0x3837_3635_3433_3231));')
P('        x = @arm_crc32(x, @as(u8, 0x39));')
P('        ok = ok and (~x == %s);' % hex(crc(b'123456789', 0xedb88320)))
P('        return ok;')
P('    }')
P('    return false;')
P('}')
P('')
P('export fn run() void {')
P('    let s = %s;' % bvec(st))
P('    let k = %s;' % bvec(key))
P('    let mut ok = true;')
P('    if @cpu_has("aes") and @cpu_has("pclmul") and @cpu_has("sse4.2") {')
P('        ok = x86(s, k);')
P('    }')
P('    if @cpu_has("aes") and @cpu_has("sha2") and @cpu_has("sha3") and @cpu_has("crc") {')
P('        ok = arm(s, k);')
P('    }')
P('    io.boolean(ok);  // expect: true')
P('}')
print('\n'.join(out))
