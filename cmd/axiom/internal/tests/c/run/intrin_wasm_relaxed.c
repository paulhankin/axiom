/* intrin_wasm_relaxed - WebAssembly vector intrinsics checked against the specification's
   definitions of their instructions (computed by
   cmd/internal/tools/gen_wasm_intrin.py, which writes this file: do not
   edit): relaxed-simd, on the inputs whose results every implementation agrees on. A mismatch prints the intrinsic and the result's bytes. */
/* wasm-only */
/* flags: --c-std=c17 --features=+relaxed-simd */
/* expect: intrin_wasm_relaxed 20 */
#include <wasm_simd128.h>
#include "host.h"

#define __L(t, o) (*(const t *)(const void *)(__pool + (o)))
static __axiom_attribute__((__aligned__(64))) unsigned char __pool[720] =
    "\x00\x3f\xc8\xa6\x08\x00\x94\xa1\x83\x03\xb9\x6a\xf6\x2d\x07\xbf\x83\x9b\x05\xee\xec\x0c\x02\x07\xbf\xd5\xdc\x00\xf3\xb7\x01\x0b"
    "\xc8\xff\x1a\x5b\x3b\x00\x10\x00\x28\x00\xa6\xd7\x21\x00\xc3\xaf\x6c\x64\x36\x46\xf1\x6d\xe6\xc1\xc5\x27\x1e\x39\xba\xc4\x69\x74"
    "\xa3\xb2\xe6\x21\xbb\xdb\xc5\x40\xaf\xbd\x6d\x19\xd3\x67\x54\x99\x60\x65\x0b\x17\x01\x32\x6f\x1f\x5f\x69\x27\x16\x11\x7d\x0d\x3e"
    "\xcb\x01\xc5\x33\x07\xf0\x38\x28\x17\x7a\xc4\xb1\xdf\x81\xf4\x7e\x04\x49\x70\x14\x45\x67\x53\x79\x3d\x47\x22\x0b\x2d\x7d\x4c\x41"
    "\x74\x77\xa3\x20\xff\xff\xff\x7f\x27\x00\x00\x00\x08\x00\x00\x00\x7e\xff\xde\x7a\x01\x00\x36\x33\x80\x7f\x08\x30\xc4\x4e\xd2\x08"
    "\x39\x1a\x1e\xc1\x08\x42\xeb\xfb\xe4\x01\x1b\x19\x01\x5b\xa1\x39\x00\x00\xff\x00\x00\x00\xff\xff\xff\x00\x00\xff\xff\xff\xff\xff"
    "\xe3\x46\xdd\x74\x3e\xc9\x0f\x00\x0a\x00\x12\x3b\x91\xf0\xb5\xe5\x01\x80\x4b\xda\x0f\x00\x74\x44\xf4\x8b\x1a\x20\x20\xdd\x9f\x88"
    "\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\x00\x00\x00\x00\xd9\xe2\x8f\xef\x2a\xbc\x71\xa5\xf1\xff\xff\xff\xb2\x4d\x75\xb7"
    "\x11\x84\x88\xc6\x01\x00\x00\x00\x73\x00\xd7\x13\x7c\x04\x42\xb6\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00"
    "\xf5\x02\x9a\x1d\xb0\xe5\x63\x54\xe7\x60\x95\x47\xec\x83\x87\x03\x01\x00\x00\x00\x00\x00\x00\x80\x84\xb9\x8f\x46\x95\x87\xcb\x1d"
    "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x4b\x73\x06\x3d\xad\x36\x89\xbc\x0d\x60\x3e\xbd\xb8\x3b\xe7\x42"
    "\x93\x08\x3b\x43\xc6\x50\x35\x3b\x37\x19\x5c\x43\x47\xe4\x98\xba\xae\xfc\x55\xbb\x34\xec\x8c\xbe\x14\x15\x94\xbe\xbf\x9f\xe3\xc1"
    "\xcb\xf3\xdd\x43\xe6\x6d\x51\x40\xe8\xdc\x24\x3e\x82\x85\xb4\xc1\x00\x00\x3c\xc2\x00\x00\x60\xc1\x00\x00\x48\xc2\x00\x00\xa0\xc1"
    "\x00\x00\x30\xc1\x00\x00\x00\xc0\x00\x00\x90\xc1\x00\x00\x28\xc2\x00\x00\x90\x41\x00\x00\xa0\x41\x00\x00\xf0\xc1\x00\x00\x1c\x42"
    "\x00\x00\x80\xbf\x00\x00\x1c\x42\x00\x00\x08\xc2\x00\x00\x40\x41\x00\x00\x80\xc0\x00\x00\x00\x42\x00\x00\x80\x40\x00\x00\x00\x41"
    "\x00\x00\x04\x42\x00\x00\xc0\x40\x00\x00\x40\x40\x00\x00\x60\x41\x30\x3e\xaa\x72\x83\x12\x55\xbf\xfc\x61\x62\x10\xaf\xa4\x66\xbf"
    "\x3a\xaa\x31\x33\xc7\xa6\xb5\x3f\x8b\x38\xdb\x64\xd9\xa3\x32\xc0\xfe\x48\xb1\xbe\x77\x44\x68\xbf\x30\x5a\x5f\xd2\xdf\x8c\xea\x3f"
    "\x60\xf2\x2e\xdf\x3c\x44\x66\x3f\x6c\xd4\xe9\x33\xe6\xb9\xd4\x3f\x00\x00\x00\x00\x00\x00\x2a\x40\x00\x00\x00\x00\x00\x00\x40\x40"
    "\x00\x00\x00\x00\x00\x00\x3d\x40\x00\x00\x00\x00\x00\x00\x3a\xc0\x00\x00\x00\x00\x00\x00\x3d\xc0\x00\x00\x00\x00\x00\x80\x45\x40"
    "\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00\x80\x40\x40\x00\x00\x00\x00\x00\x00\x36\x40\x00\x00\x00\x00\x00\x00\x20\xc0"
    "\x00\x00\x00\x00\x00\x80\x45\xc0\x00\x00\x00\x00\x00\x80\x48\xc0\x57\x2d\x05\x4e\x93\xb5\xdc\x4e\x28\xd6\xb3\x4d\xef\x94\x83\xcd"
    "\x00\x00\x80\x95\x63\x7a\xc0\xc1\x00\x00\x40\x44\x5d\xb4\xd5\xc1\x6d\x0f\x0b\x4f\xfc\xb0\x9e\x4e\xbc\x73\x36\x4f\x2f\x19\x5d\x4f"
    "\x00\x00\x60\x8a\x8a\x2d\xea\x41\x00\x00\xc0\xcc\x9c\x84\xd4\x41";

static int __checks;
static int __fails;
static char __line[512];
static size_t __len;
static unsigned char __out[64];

static void say(const char *s) {
    while (*s != 0 && __len < sizeof __line) {
        __line[__len++] = *s++;
    }
}

static void endl(void) {
    print_str(__line, __len);
    __len = 0;
}

static void say_n(int v) {
    char b[16];
    int k = 15;
    b[15] = 0;
    do {
        b[--k] = (char)('0' + v % 10);
        v /= 10;
    } while (v != 0);
    say(b + k);
}

static void hex(const unsigned char *p, int n) {
    static const char d[] = "0123456789abcdef";
    char b[3];
    int i;
    b[2] = 0;
    for (i = 0; i < n; i++) {
        b[0] = d[p[i] >> 4];
        b[1] = d[p[i] & 15];
        say(b);
    }
}

static void __fill(void) {
    int i;
    for (i = 0; i < 64; i++) {
        __out[i] = 0x5a;
    }
}

/* A floating lane of w bytes at p: its value, and whether it is a NaN */
static double lane(const unsigned char *p, int w, int *nan) {
    unsigned long long b = 0;
    int i;
    for (i = w - 1; i >= 0; i--) {
        b = b << 8 | p[i];
    }
    if (w == 2) {
        int e = (int)(b >> 10 & 31);
        double m = (double)(b & 1023);
        double v;
        *nan = e == 31 && (b & 1023) != 0;
        if (e == 31) {
            v = 1e300 * 1e300;
        } else if (e == 0) {
            v = m / 1024.0 / 16384.0;
        } else {
            v = (1.0 + m / 1024.0);
            while (e > 15) {
                v *= 2.0;
                e--;
            }
            while (e < 15) {
                v /= 2.0;
                e++;
            }
        }
        return (b >> 15) ? -v : v;
    }
    if (w == 4) {
        union { unsigned int u; float f; } x;
        x.u = (unsigned int)b;
        *nan = x.f != x.f;
        return x.f;
    }
    {
        union { unsigned long long u; double f; } x;
        x.u = b;
        *nan = x.f != x.f;
        return x.f;
    }
}

/* The result's n bytes at r against w's: mode 0 exact; 1, 2, 3 floating lanes of 2, 4, 8 bytes, a NaN for a NaN;
   4, 5, 6 those within a relative 2^-7 */
static void chk(const char *name, const void *r, int n, const char *w, int mode) {
    const unsigned char *p = (const unsigned char *)r;
    const unsigned char *q = (const unsigned char *)w;
    int i;
    int bad = 0;
    __checks++;
    for (i = 0; i < n; i++) {
        if (p[i] != q[i]) {
            bad = 1;
        }
    }
    if (bad && mode != 0) {
        int lw = mode == 1 || mode == 4 ? 2 : mode == 2 || mode == 5 ? 4 : 8;
        bad = 0;
        for (i = 0; i + lw <= n; i += lw) {
            int an;
            int bn;
            double x = lane(p + i, lw, &an);
            double y = lane(q + i, lw, &bn);
            int j;
            int same = 1;
            for (j = 0; j < lw; j++) {
                if (p[i + j] != q[i + j]) {
                    same = 0;
                }
            }
            if (same || (an && bn)) {
                continue;
            }
            if (mode >= 4 && !an && !bn && y != 0 && x / y > 1 - 1.0 / 128 && x / y < 1 + 1.0 / 128) {
                continue;
            }
            bad = 1;
        }
    }
    if (bad) {
        __fails++;
        say(name);
        say(": ");
        hex(p, n);
        say(" expected ");
        hex(q, n);
        endl();
    }
}

/* (the inputs, written where the compiler cannot see: no result is folded) */
static void (*volatile __touch)(unsigned char *) = 0;

static void run0(void) {
    { v128_t __r = wasm_i8x16_relaxed_swizzle(__L(v128_t, 0), __L(v128_t, 16)); chk("wasm_i8x16_relaxed_swizzle", &__r, 16, "\x00\x00\x00\x00\x00\xf6\xc8\xa1\x00\x00\x00\x00\x00\x00\x3f\x6a", 0); }
    { v128_t __r = wasm_i16x8_relaxed_q15mulr(__L(v128_t, 32), __L(v128_t, 48)); chk("wasm_i16x8_relaxed_q15mulr", &__r, 16, "\xd4\xff\xf9\x31\x33\x00\xf8\xff\x0c\x00\xfe\xed\xf1\xff\x07\xb7", 0); }
    { v128_t __r = wasm_i16x8_relaxed_dot_i8x16_i7x16(__L(v128_t, 64), __L(v128_t, 80)); chk("wasm_i16x8_relaxed_dot_i8x16_i7x16", &__r, 16, "\x5a\xbe\xd9\x01\x81\xf8\x2b\xee\x76\xc6\xc1\x12\x4e\x2f\x52\xeb", 0); }
    { v128_t __r = wasm_i32x4_relaxed_dot_i8x16_i7x16_add(__L(v128_t, 96), __L(v128_t, 112), __L(v128_t, 128)); chk("wasm_i32x4_relaxed_dot_i8x16_i7x16_add", &__r, 16, "\x15\x61\xa3\x20\x82\x20\x00\x80\x1b\x1c\x00\x00\xa6\xd8\xff\xff", 0); }
    { v128_t __r = wasm_i8x16_relaxed_laneselect(__L(v128_t, 144), __L(v128_t, 160), __L(v128_t, 176)); chk("wasm_i8x16_relaxed_laneselect", &__r, 16, "\x39\x1a\xde\xc1\x08\x42\x36\x33\x80\x01\x1b\x30\xc4\x4e\xd2\x08", 0); }
    { v128_t __r = wasm_i16x8_relaxed_laneselect(__L(v128_t, 192), __L(v128_t, 208), __L(v128_t, 224)); chk("wasm_i16x8_relaxed_laneselect", &__r, 16, "\xe3\x46\x4b\xda\x0f\x00\x74\x44\xf4\x8b\x12\x3b\x20\xdd\x9f\x88", 0); }
    { v128_t __r = wasm_i32x4_relaxed_laneselect(__L(v128_t, 240), __L(v128_t, 256), __L(v128_t, 272)); chk("wasm_i32x4_relaxed_laneselect", &__r, 16, "\x11\x84\x88\xc6\x01\x00\x00\x00\xf1\xff\xff\xff\x7c\x04\x42\xb6", 0); }
    { v128_t __r = wasm_i64x2_relaxed_laneselect(__L(v128_t, 288), __L(v128_t, 304), __L(v128_t, 320)); chk("wasm_i64x2_relaxed_laneselect", &__r, 16, "\x01\x00\x00\x00\x00\x00\x00\x80\xe7\x60\x95\x47\xec\x83\x87\x03", 0); }
    { v128_t __r = wasm_f32x4_relaxed_min(__L(v128_t, 336), __L(v128_t, 352)); chk("wasm_f32x4_relaxed_min", &__r, 16, "\x4b\x73\x06\x3d\xad\x36\x89\xbc\x0d\x60\x3e\xbd\x47\xe4\x98\xba", 2); }
    { v128_t __r = wasm_f32x4_relaxed_max(__L(v128_t, 368), __L(v128_t, 384)); chk("wasm_f32x4_relaxed_max", &__r, 16, "\xcb\xf3\xdd\x43\xe6\x6d\x51\x40\xe8\xdc\x24\x3e\x82\x85\xb4\xc1", 2); }
    { v128_t __r = wasm_f32x4_relaxed_madd(__L(v128_t, 400), __L(v128_t, 416), __L(v128_t, 432)); chk("wasm_f32x4_relaxed_madd", &__r, 16, "\x00\xc0\x05\x44\x00\x00\x40\x42\x00\x80\x59\x44\x00\xc0\x5b\x44", 2); }
    { v128_t __r = wasm_f32x4_relaxed_nmadd(__L(v128_t, 448), __L(v128_t, 464), __L(v128_t, 480)); chk("wasm_f32x4_relaxed_nmadd", &__r, 16, "\x00\x00\xe8\x41\x00\x40\x9b\xc4\x00\x00\x0b\x43\x00\x00\xa4\xc2", 2); }
    { v128_t __r = wasm_f64x2_relaxed_min(__L(v128_t, 496), __L(v128_t, 512)); chk("wasm_f64x2_relaxed_min", &__r, 16, "\x30\x3e\xaa\x72\x83\x12\x55\xbf\x8b\x38\xdb\x64\xd9\xa3\x32\xc0", 3); }
    { v128_t __r = wasm_f64x2_relaxed_max(__L(v128_t, 528), __L(v128_t, 544)); chk("wasm_f64x2_relaxed_max", &__r, 16, "\x60\xf2\x2e\xdf\x3c\x44\x66\x3f\x30\x5a\x5f\xd2\xdf\x8c\xea\x3f", 3); }
    { v128_t __r = wasm_f64x2_relaxed_madd(__L(v128_t, 560), __L(v128_t, 576), __L(v128_t, 592)); chk("wasm_f64x2_relaxed_madd", &__r, 16, "\x00\x00\x00\x00\x00\xc0\x75\x40\x00\x00\x00\x00\x00\xa8\x88\xc0", 3); }
    { v128_t __r = wasm_f64x2_relaxed_nmadd(__L(v128_t, 608), __L(v128_t, 624), __L(v128_t, 640)); chk("wasm_f64x2_relaxed_nmadd", &__r, 16, "\x00\x00\x00\x00\x00\xc0\x55\xc0\x00\x00\x00\x00\x00\xe0\x6a\x40", 3); }
}

static void run1(void) {
    { v128_t __r = wasm_i32x4_relaxed_trunc_f32x4(__L(v128_t, 656)); chk("wasm_i32x4_relaxed_trunc_f32x4", &__r, 16, "\xc0\x55\x4b\x21\x80\xc9\x5a\x6e\x00\xc5\x7a\x16\x20\x62\x8d\xef", 0); }
    { v128_t __r = wasm_i32x4_relaxed_trunc_f64x2_zero(__L(v128_t, 672)); chk("wasm_i32x4_relaxed_trunc_f64x2_zero", &__r, 16, "\xd5\x38\x0b\xdf\xef\x8a\x2e\xa9\x00\x00\x00\x00\x00\x00\x00\x00", 0); }
    { v128_t __r = wasm_u32x4_relaxed_trunc_f32x4(__L(v128_t, 688)); chk("wasm_u32x4_relaxed_trunc_f32x4", &__r, 16, "\x00\x6d\x0f\x8b\x00\x7e\x58\x4f\x00\xbc\x73\xb6\x00\x2f\x19\xdd", 0); }
    { v128_t __r = wasm_u32x4_relaxed_trunc_f64x2_zero(__L(v128_t, 704)); chk("wasm_u32x4_relaxed_trunc_f64x2_zero", &__r, 16, "\x53\x54\x6c\xd1\x33\x73\x12\x52\x00\x00\x00\x00\x00\x00\x00\x00", 0); }
}

int main(void) {
    if (__touch != 0) {
        __touch(__pool);
    }
    run0();
    run1();
    if (__fails == 0) {
        say("intrin_wasm_relaxed ");
        say_n(__checks);
        endl();
    }
    return 0;
}
