// =============================================================================
// std/crypto/kdf.ph - password-based key derivation: PBKDF2 (RFC 8018)
// and scrypt (RFC 7914). For new password storage prefer Argon2id
// (std/crypto/argon2), or scrypt; PBKDF2 is here for compatibility.
//
//     kdf.pbkdf2(sha2.Algorithm.SHA256, password, salt, 600000, key[..32]);
//     kdf.scrypt(password, salt, 1 << 15, 8, 1, key[..32])!;
// =============================================================================

import "../mem.ph" as mem;
import "hmac.ph" as hmac;
import "sha2.ph" as sha2;

error { InvalidArgument, TooExpensive, OutOfMemory }

/// PBKDF2 with HMAC over a sha2.Algorithm (SHA1, SHA256, SHA384, SHA512):
/// fills out with derived key bytes. iterations >= 1.
pub fn pbkdf2(kind: sha2.Algorithm, password: []u8, salt: []u8, iterations: u32, out: []mut u8) void {
    if iterations == 0 {
        @trap();
    }
    let hl = sha2.digest_size(kind);
    let base = hmac.Hmac.init(kind, password);
    let mut u: [64]u8 = [0; 64];
    let mut t: [64]u8 = [0; 64];
    let mut block: u32 = 1;
    let mut o: usize = 0;
    while o < out.len {
        let mut m = base;
        m.update(salt);
        let be: [4]u8 = [@truncate(u8, block >> 24), @truncate(u8, block >> 16), @truncate(u8, block >> 8),
                         @truncate(u8, block)];
        m.update(be[..]);
        m.final(u[..hl]);
        @mem_copy(t[..hl], u[..hl]);
        for _i in @as(u32, 1)..iterations {
            let mut n = base;
            n.update(u[..hl]);
            n.final(u[..hl]);
            for k in @as(usize, 0)..hl {
                t[k] ^= u[k];
            }
        }
        let take = @min(hl, out.len - o);
        @mem_copy(out[o..o + take], t[..take]);
        o += take;
        block += 1;
    }
}

// -----------------------------------------------------------------------------
// scrypt
// -----------------------------------------------------------------------------

/// The most memory scrypt may use (128 * r * (n + p) bytes): 1 GiB.
pub const SCRYPT_MAX_MEMORY: u64 = 1 << 30;

fn salsa20_8(b: *mut [16]u32) void {
    let mut x = b.*;
    for _r in @as(usize, 0)..4 {
        x[4] ^= @rotl(x[0] +% x[12], 7);
        x[8] ^= @rotl(x[4] +% x[0], 9);
        x[12] ^= @rotl(x[8] +% x[4], 13);
        x[0] ^= @rotl(x[12] +% x[8], 18);
        x[9] ^= @rotl(x[5] +% x[1], 7);
        x[13] ^= @rotl(x[9] +% x[5], 9);
        x[1] ^= @rotl(x[13] +% x[9], 13);
        x[5] ^= @rotl(x[1] +% x[13], 18);
        x[14] ^= @rotl(x[10] +% x[6], 7);
        x[2] ^= @rotl(x[14] +% x[10], 9);
        x[6] ^= @rotl(x[2] +% x[14], 13);
        x[10] ^= @rotl(x[6] +% x[2], 18);
        x[3] ^= @rotl(x[15] +% x[11], 7);
        x[7] ^= @rotl(x[3] +% x[15], 9);
        x[11] ^= @rotl(x[7] +% x[3], 13);
        x[15] ^= @rotl(x[11] +% x[7], 18);
        x[1] ^= @rotl(x[0] +% x[3], 7);
        x[2] ^= @rotl(x[1] +% x[0], 9);
        x[3] ^= @rotl(x[2] +% x[1], 13);
        x[0] ^= @rotl(x[3] +% x[2], 18);
        x[6] ^= @rotl(x[5] +% x[4], 7);
        x[7] ^= @rotl(x[6] +% x[5], 9);
        x[4] ^= @rotl(x[7] +% x[6], 13);
        x[5] ^= @rotl(x[4] +% x[7], 18);
        x[11] ^= @rotl(x[10] +% x[9], 7);
        x[8] ^= @rotl(x[11] +% x[10], 9);
        x[9] ^= @rotl(x[8] +% x[11], 13);
        x[10] ^= @rotl(x[9] +% x[8], 18);
        x[12] ^= @rotl(x[15] +% x[14], 7);
        x[13] ^= @rotl(x[12] +% x[15], 9);
        x[14] ^= @rotl(x[13] +% x[12], 13);
        x[15] ^= @rotl(x[14] +% x[13], 18);
    }
    for i in @as(usize, 0)..16 {
        b[i] +%= x[i];
    }
}

// BlockMix: b (2r blocks of 16 words) into y, with even outputs first.
fn block_mix(b: []u32, y: []mut u32, r: usize) void {
    let mut x: [16]u32 = [0; 16];
    for k in @as(usize, 0)..16 {
        x[k] = b[(((2 * r) - 1) * 16) + k];
    }
    for i in @as(usize, 0)..2 * r {
        for k in @as(usize, 0)..16 {
            x[k] ^= b[(i * 16) + k];
        }
        salsa20_8(&mut x);
        let dst = ((i / 2) + ((i % 2) * r)) * 16;
        for k in @as(usize, 0)..16 {
            y[dst + k] = x[k];
        }
    }
}

// ROMix on one 128r-byte block (as words), with v (n blocks) and t scratch.
fn ro_mix(block: []mut u32, n: usize, r: usize, v: []mut u32, t: []mut u32) void {
    let w = 32 * r;  // words per block
    let x = block;
    for i in @as(usize, 0)..n {
        @mem_copy(v[i * w..(i + 1) * w], x);
        block_mix(x, t, r);
        @mem_copy(x, t[..w]);
    }
    for _i in @as(usize, 0)..n {
        let j = @as(usize, x[(((2 * r) - 1) * 16)]) & (n - 1);
        for k in @as(usize, 0)..w {
            x[k] ^= v[(j * w) + k];
        }
        block_mix(x, t, r);
        @mem_copy(x, t[..w]);
    }
}

/// scrypt: fills out with key bytes derived from password and salt. n (a
/// power of two >= 2) sets the CPU and memory cost, r the block size and p
/// the parallelism; TooExpensive if it would need more than
/// SCRYPT_MAX_MEMORY.
pub fn scrypt(password: []u8, salt: []u8, n: u64, r: u32, p: u32, out: []mut u8) !void {
    if (n < 2) or ((n & (n - 1)) != 0) or (r == 0) or (p == 0) or ((@as(u64, r) * @as(u64, p)) >= (1 << 30)) {
        return error.InvalidArgument;
    }
    let mem_need = (128 * @as(u64, r)) * (n + @as(u64, p));
    if (mem_need > SCRYPT_MAX_MEMORY) or (n > 0xffffffff) {
        return error.TooExpensive;
    }
    let rr = @as(usize, r);
    let nn = @truncate(usize, n);
    let w = 32 * rr;
    let bytes = mem.alloc.[u8](128 * rr * @as(usize, p))!;
    defer mem.free(bytes);
    pbkdf2(sha2.Algorithm.SHA256, password, salt, 1, bytes);
    let b = mem.alloc.[u32](w * @as(usize, p))!;
    defer mem.free(b);
    for i in @as(usize, 0)..b.len {
        let q = bytes[4 * i..(4 * i) + 4];
        b[i] = ((@as(u32, q[0]) | (@as(u32, q[1]) << 8)) | (@as(u32, q[2]) << 16)) | (@as(u32, q[3]) << 24);
    }
    let v = mem.alloc.[u32](w * nn)!;
    defer mem.free(v);
    let t = mem.alloc.[u32](w)!;
    defer mem.free(t);
    for i in @as(usize, 0)..@as(usize, p) {
        ro_mix(b[i * w..(i + 1) * w], nn, rr, v, t);
    }
    for i in @as(usize, 0)..b.len {
        bytes[4 * i] = @truncate(u8, b[i]);
        bytes[(4 * i) + 1] = @truncate(u8, b[i] >> 8);
        bytes[(4 * i) + 2] = @truncate(u8, b[i] >> 16);
        bytes[(4 * i) + 3] = @truncate(u8, b[i] >> 24);
    }
    pbkdf2(sha2.Algorithm.SHA256, password, bytes, 1, out);
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

fn hex(d: []u8, out: []mut u8) []u8 {
    let digits = "0123456789abcdef";
    for i in @as(usize, 0)..d.len {
        out[2 * i] = digits[@as(usize, d[i] >> 4)];
        out[(2 * i) + 1] = digits[@as(usize, d[i] & 15)];
    }
    return out[..2 * d.len];
}

test "pbkdf2" {
    let mut k: [100]u8 = [0; 100];
    let mut h: [200]u8 = [0; 200];
    pbkdf2(sha2.Algorithm.SHA256, "password", "salt", 4096, k[..32]);
    @expect_eq(hex(k[..32], h[..]), "c5e478d59288c841aa530db6845c4c8d962893a001ce4e11a4963873aa98134a");
    pbkdf2(sha2.Algorithm.SHA1, "password", "salt", 2, k[..20]);
    @expect_eq(hex(k[..20], h[..]), "ea6c014dc72d6f8ccd1ed92ace1d41f0d8de8957");
    pbkdf2(sha2.Algorithm.SHA512, "passwordPASSWORDpassword", "saltSALTsaltSALTsaltSALTsaltSALTsalt", 1000, k[..]);
    @expect_eq(hex(k[..], h[..]),
               "0e28f3efa802a2f0cd3b4ace5e3d9afadb7c2dccc5ef10eedb8a6564dfb0c9a63b6f46b1e150587b9fe7875cfaf999d00b454bb7d74295c60df1bbe5f8f36da188271db22110efda5cc9eeafb0ab29697849379903421d54eee3949344c72873d6ce9742");
}

test "scrypt (rfc 7914 vectors)" {
    let mut k: [64]u8 = [0; 64];
    let mut h: [128]u8 = [0; 128];
    scrypt("", "", 16, 1, 1, k[..])!;
    @expect_eq(hex(k[..], h[..]),
               "77d6576238657b203b19ca42c18a0497f16b4844e3074ae8dfdffa3fede21442fcd0069ded0948f8326a753a0fc81f17e8d3e0fb2e0d3628cf35e20c38d18906");
    scrypt("password", "NaCl", 1024, 8, 16, k[..])!;
    @expect_eq(hex(k[..], h[..]),
               "fdbabe1c9d3472007856e7190d01e9fe7c6ad7cbc8237830e77376634b3731622eaf30d92e22a3886ff109279d9830dac727afb94a83ee6d8360cbdfa2cc0640");
    @expect(fails(scrypt("p", "s", 1000, 8, 1, k[..])));     // not a power of two
    @expect(fails(scrypt("p", "s", 1 << 40, 8, 1, k[..])));  // far too much memory
    @expect(fails(scrypt("p", "s", 16, 0, 1, k[..])));
}

fn fails(r: !void) bool {
    r catch return true;
    return false;
}
