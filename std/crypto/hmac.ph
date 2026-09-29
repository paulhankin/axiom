// =============================================================================
// std/crypto/hmac.ph - HMAC (RFC 2104) and HKDF (RFC 5869) over SHA-2.
//
//     let mut mac: [32]u8 = [0; 32];
//     hmac.hmac(sha2.SHA256, key, message, mac[..]);
// =============================================================================

import "sha2.ph" as sha2;

pub struct Hmac {
    inner: sha2.Hasher,
    outer: sha2.Hasher,

    pub fn init(kind: u8, key: []u8) Hmac {
        let mut k: [128]u8 = [0; 128];
        let probe = sha2.Hasher.init(kind);
        let bs = probe.block_size();
        if key.len > bs {
            sha2.hash(kind, key, k[..]);
        } else {
            @mem_copy(k[..key.len], key);
        }
        let mut ipad: [128]u8 = [0; 128];
        let mut opad: [128]u8 = [0; 128];
        for i in 0..bs {
            ipad[i] = k[i] ^ 0x36;
            opad[i] = k[i] ^ 0x5c;
        }
        let mut h = Hmac.{ inner: sha2.Hasher.init(kind), outer: sha2.Hasher.init(kind) };
        h.inner.update(ipad[..bs]);
        h.outer.update(opad[..bs]);
        return h;
    }

    pub fn update(self: *mut Hmac, data: []u8) void {
        self.inner.update(data);
    }

    /// Writes the MAC (the hash's size) to out.
    pub fn final(self: *mut Hmac, out: []mut u8) void {
        let mut d: [64]u8 = [0; 64];
        let n = self.inner.size();
        self.inner.final(d[..]);
        self.outer.update(d[..n]);
        self.outer.final(out);
    }
}

/// HMAC of message under key, written to out (the hash's size).
pub fn hmac(kind: u8, key: []u8, message: []u8, out: []mut u8) void {
    let mut h = Hmac.init(kind, key);
    h.update(message);
    h.final(out);
}

/// HKDF-Extract: a pseudorandom key (the hash's size) from input keying
/// material and a salt.
pub fn extract(kind: u8, salt: []u8, ikm: []u8, prk: []mut u8) void {
    let zeros: [64]u8 = [0; 64];
    let mut s = salt;
    if s.len == 0 {
        s = zeros[..sha2.digest_size(kind)];
    }
    hmac(kind, s, ikm, prk);
}

/// HKDF-Expand: fills out (at most 255 hash lengths) from a pseudorandom key
/// and context info.
pub fn expand(kind: u8, prk: []u8, info: []u8, out: []mut u8) void {
    let n = sha2.digest_size(kind);
    if out.len > (255 * n) {
        @trap();  // more than HKDF can make (RFC 5869): a caller's mistake
    }
    let mut t: [64]u8 = [0; 64];
    let mut tlen: usize = 0;
    let mut done: usize = 0;
    let mut counter: u8 = 1;
    while done < out.len {
        let mut h = Hmac.init(kind, prk);
        h.update(t[..tlen]);
        h.update(info);
        let c: [1]u8 = [counter];
        h.update(c[..]);
        h.final(t[..]);
        tlen = n;
        let k = @min(n, out.len - done);
        @mem_copy(out[done..done + k], t[..k]);
        done += k;
        counter +%= 1;
    }
}

fn unhex(s: []u8, out: []mut u8) []u8 {
    for i in 0..s.len / 2 {
        let hi = s[2 * i];
        let lo = s[(2 * i) + 1];
        out[i] = (hexv(hi) << 4) | hexv(lo);
    }
    return out[..s.len / 2];
}

fn hexv(c: u8) u8 {
    if c <= '9' {
        return c - '0';
    }
    return (c | 0x20) - 'a' + 10;
}

test "hmac (RFC 4231 case 2)" {
    let mut out: [64]u8 = [0; 64];
    let mut want: [64]u8 = [0; 64];
    hmac(sha2.SHA256, "Jefe", "what do ya want for nothing?", out[..32]);
    @expect_eq(out[..32], unhex("5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843", want[..]));
    hmac(sha2.SHA512, "Jefe", "what do ya want for nothing?", out[..]);
    @expect_eq(out[..],
               unhex("164b7a7bfcf819e2e395fbe73b56e0a387bd64222e831fd610270cd7ea2505549758bf75c05a994a6d034f65f8f0e6fdcaeab1a34d4a6b4b636e070a38bce737",
                     want[..]));
    hmac(sha2.SHA384, "Jefe", "what do ya want for nothing?", out[..48]);
    @expect_eq(out[..48],
               unhex("af45d2e376484031617f78d2b58a6b1b9c7ef464f5a01b47e42ec3736322445e8e2240ca5e69e2c78b3239ecfab21649",
                     want[..]));
    // a key longer than the block
    let key: [131]u8 = [0xaa; 131];
    hmac(sha2.SHA256, key[..], "Test Using Larger Than Block-Size Key - Hash Key First", out[..32]);
    @expect_eq(out[..32], unhex("60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54", want[..]));
}

test "hkdf (RFC 5869 case 1)" {
    let ikm: [22]u8 = [0x0b; 22];
    let mut salt: [13]u8 = [0; 13];
    for i in @as(usize, 0)..13 {
        salt[i] = @truncate(u8, i);
    }
    let mut info: [10]u8 = [0; 10];
    for i in @as(usize, 0)..10 {
        info[i] = 0xf0 + @truncate(u8, i);
    }
    let mut prk: [32]u8 = [0; 32];
    let mut want: [64]u8 = [0; 64];
    extract(sha2.SHA256, salt[..], ikm[..], prk[..]);
    @expect_eq(prk[..], unhex("077709362c2e32df0ddc3f0dc47bba6390b6c73bb50f9c3122ec844ad7c2b3e5", want[..]));
    let mut okm: [42]u8 = [0; 42];
    expand(sha2.SHA256, prk[..], info[..], okm[..]);
    @expect_eq(okm[..], unhex("3cb25f25faacd57a90434f64d0362f2a2d2d0a90cf1a5a4c5db02d56ecc4c5bf34007208d5b887185865",
                              want[..]));
}
