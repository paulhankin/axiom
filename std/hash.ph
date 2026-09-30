// =============================================================================
// std/hash.ph - hash and equality functions for std/collections/map.
//
// The hashes are keyed with a random key chosen once per process, so that
// whoever supplies a Map's keys (a JSON document, HTTP headers, ...) cannot
// pick ones that collide: byte strings go through SipHash-1-3, integers
// through a keyed splitmix64 finalizer. They are not cryptographic digests,
// and they differ from run to run.
// =============================================================================

import "crypto/random.ph" as random;
import "sync.ph" as sync;
import "time.ph" as time;

let mut key_once: sync.Once = sync.Once.{ state: 0 };
let mut k0: u64 = 0;
let mut k1: u64 = 0;

fn key() void {
    key_once.call(choose_key, @as(u8, 0));
}

fn choose_key(_u: u8) void {
    let mut b: [16]u8 = [0; 16];
    random.fill(b[..]) catch {
        // (no system randomness: the clock is a poor key, but a key)
        let t = @bit_cast(u64, time.Instant.now().ns);
        for i in @as(usize, 0)..16 {
            b[i] = @truncate(u8, splitmix(t +% @as(u64, i)) >> 17);
        }
    };
    k0 = load64(b[..8]);
    k1 = load64(b[8..]);
}

fn load64(b: []u8) u64 {
    return @as(u64, b[0]) | (@as(u64, b[1]) << 8) | (@as(u64, b[2]) << 16) | (@as(u64, b[3]) << 24) | (
        @as(u64, b[4]) << 32) | (@as(u64, b[5]) << 40) | (@as(u64, b[6]) << 48) | (@as(u64, b[7]) << 56);
}

fn splitmix(x: u64) u64 {
    let mut z = x +% 0x9E3779B97F4A7C15;
    z = (z ^ (z >> 30)) *% 0xBF58476D1CE4E5B9;
    z = (z ^ (z >> 27)) *% 0x94D049BB133111EB;
    return z ^ (z >> 31);
}

/// A keyed splitmix64 finalizer: fast and well mixed, for integer keys.
pub fn hash_u64(x: u64) u64 {
    key();
    return splitmix(x ^ k0);
}

pub fn hash_u32(x: u32) u64 {
    return hash_u64(@as(u64, x));
}

pub fn hash_i64(x: i64) u64 {
    return hash_u64(@bit_cast(u64, x));
}

pub fn hash_i32(x: i32) u64 {
    return hash_u64(@bit_cast(u64, @as(i64, x)));
}

pub fn hash_usize(x: usize) u64 {
    return hash_u64(@as(u64, x));
}

/// SipHash-1-3 of the bytes under the process's key.
pub fn hash_bytes(s: []u8) u64 {
    key();
    let mut v0 = k0 ^ 0x736f6d6570736575;
    let mut v1 = k1 ^ 0x646f72616e646f6d;
    let mut v2 = k0 ^ 0x6c7967656e657261;
    let mut v3 = k1 ^ 0x7465646279746573;
    let n = s.len;
    let end = n - (n % 8);
    let mut i: usize = 0;
    while i < end {
        let m = load64(s[i..i + 8]);
        v3 ^= m;
        // one compression round
        v0 +%= v1;
        v1 = @rotl(v1, 13) ^ v0;
        v0 = @rotl(v0, 32);
        v2 +%= v3;
        v3 = @rotl(v3, 16) ^ v2;
        v0 +%= v3;
        v3 = @rotl(v3, 21) ^ v0;
        v2 +%= v1;
        v1 = @rotl(v1, 17) ^ v2;
        v2 = @rotl(v2, 32);
        v0 ^= m;
        i += 8;
    }
    let mut b = @as(u64, n & 0xff) << 56;
    let mut sh: u64 = 0;
    while i < n {
        b |= @as(u64, s[i]) << sh;
        sh += 8;
        i += 1;
    }
    v3 ^= b;
    v0 +%= v1;
    v1 = @rotl(v1, 13) ^ v0;
    v0 = @rotl(v0, 32);
    v2 +%= v3;
    v3 = @rotl(v3, 16) ^ v2;
    v0 +%= v3;
    v3 = @rotl(v3, 21) ^ v0;
    v2 +%= v1;
    v1 = @rotl(v1, 17) ^ v2;
    v2 = @rotl(v2, 32);
    v0 ^= b;
    // three finalization rounds
    v2 ^= 0xff;
    for _r in @as(usize, 0)..3 {
        v0 +%= v1;
        v1 = @rotl(v1, 13) ^ v0;
        v0 = @rotl(v0, 32);
        v2 +%= v3;
        v3 = @rotl(v3, 16) ^ v2;
        v0 +%= v3;
        v3 = @rotl(v3, 21) ^ v0;
        v2 +%= v1;
        v1 = @rotl(v1, 17) ^ v2;
        v2 = @rotl(v2, 32);
    }
    return v0 ^ v1 ^ v2 ^ v3;
}

pub fn eq_u64(a: u64, b: u64) bool {
    return a == b;
}

pub fn eq_u32(a: u32, b: u32) bool {
    return a == b;
}

pub fn eq_i64(a: i64, b: i64) bool {
    return a == b;
}

pub fn eq_i32(a: i32, b: i32) bool {
    return a == b;
}

pub fn eq_usize(a: usize, b: usize) bool {
    return a == b;
}

pub fn eq_bytes(a: []u8, b: []u8) bool {
    return @mem_eq(a, b);
}

// -----------------------------------------------------------------------------
// Tests (phalange test std/hash.ph)
// -----------------------------------------------------------------------------

test "hashes are deterministic and spread" {
    @expect_eq(hash_u64(42), hash_u64(42));
    @expect(hash_u64(1) != hash_u64(2));
    @expect_eq(hash_bytes("abc"), hash_bytes("abc"));
    @expect(hash_bytes("abc") != hash_bytes("abd"));
    @expect(eq_bytes("xy", "xy"));
    @expect(not eq_bytes("xy", "x"));
}

test "siphash-1-3 known answer" {
    // the reference test vector's key (00..0f) and message (00..0e)
    let saved0 = k0;
    let saved1 = k1;
    key();
    k0 = 0x0706050403020100;
    k1 = 0x0f0e0d0c0b0a0908;
    let mut msg: [15]u8 = [0; 15];
    for i in @as(usize, 0)..15 {
        msg[i] = @truncate(u8, i);
    }
    let h = hash_bytes(msg[..]);
    k0 = saved0;
    k1 = saved1;
    @expect_eq(h, 0xd320d86d2a519956);
}
