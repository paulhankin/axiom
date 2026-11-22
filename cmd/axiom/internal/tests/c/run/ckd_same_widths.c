/* flags: --c-std=gnu17 */
/* __builtin_add_overflow, __builtin_sub_overflow and __builtin_mul_overflow
   with operands and result of the same type, of every width, signed and
   unsigned, at the edges of the ranges: the number of overflows and a hash
   of the results of every pair, with the overflow tested by a branch and
   counted as a value. (Generated with the expectations from the exact
   results.) */
#include "host.h"
static volatile unsigned char v_u8[12] = { 0U, 1U, 2U, 3U, 255U, 254U, 127U, 128U, 85U, 86U, 15U, 16U };
static volatile signed char v_i8[25] = { 0, 1, 2, 3, 127, 126, 63, 64, 42, 43, 11, 12, 16, 15, -1, -2, (-127 - 1), -127, -64, -65, -11, -12, -16, -43, -44 };
static volatile unsigned short v_u16[12] = { 0U, 1U, 2U, 3U, 65535U, 65534U, 32767U, 32768U, 21845U, 21846U, 255U, 256U };
static volatile short v_i16[25] = { 0, 1, 2, 3, 32767, 32766, 16383, 16384, 10922, 10923, 181, 182, 256, 255, -1, -2, (-32767 - 1), -32767, -16384, -16385, -181, -182, -256, -10923, -10924 };
static volatile unsigned v_u32[12] = { 0U, 1U, 2U, 3U, 4294967295U, 4294967294U, 2147483647U, 2147483648U, 1431655765U, 1431655766U, 65535U, 65536U };
static volatile int v_i32[25] = { 0, 1, 2, 3, 2147483647, 2147483646, 1073741823, 1073741824, 715827882, 715827883, 46340, 46341, 65536, 65535, -1, -2, (-2147483647 - 1), -2147483647, -1073741824, -1073741825, -46340, -46341, -65536, -715827883, -715827884 };
static volatile unsigned long long v_u64[12] = { 0ULL, 1ULL, 2ULL, 3ULL, 18446744073709551615ULL, 18446744073709551614ULL, 9223372036854775807ULL, 9223372036854775808ULL, 6148914691236517205ULL, 6148914691236517206ULL, 4294967295ULL, 4294967296ULL };
static volatile long long v_i64[25] = { 0LL, 1LL, 2LL, 3LL, 9223372036854775807LL, 9223372036854775806LL, 4611686018427387903LL, 4611686018427387904LL, 3074457345618258602LL, 3074457345618258603LL, 3037000499LL, 3037000500LL, 4294967296LL, 4294967295LL, -1LL, -2LL, (-9223372036854775807 - 1), -9223372036854775807LL, -4611686018427387904LL, -4611686018427387905LL, -3037000499LL, -3037000500LL, -4294967296LL, -3074457345618258603LL, -3074457345618258604LL };
static void add_u8(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned char a = v_u8[i], b = v_u8[j], r;
            if (__builtin_add_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + r;
        }
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned char r;
            m += __builtin_add_overflow(v_u8[i], v_u8[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void sub_u8(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned char a = v_u8[i], b = v_u8[j], r;
            if (__builtin_sub_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + r;
        }
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned char r;
            m += __builtin_sub_overflow(v_u8[i], v_u8[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void mul_u8(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned char a = v_u8[i], b = v_u8[j], r;
            if (__builtin_mul_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + r;
        }
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned char r;
            m += __builtin_mul_overflow(v_u8[i], v_u8[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void add_i8(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            signed char a = v_i8[i], b = v_i8[j], r;
            if (__builtin_add_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + (unsigned long long)(long long)r;
        }
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            signed char r;
            m += __builtin_add_overflow(v_i8[i], v_i8[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void sub_i8(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            signed char a = v_i8[i], b = v_i8[j], r;
            if (__builtin_sub_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + (unsigned long long)(long long)r;
        }
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            signed char r;
            m += __builtin_sub_overflow(v_i8[i], v_i8[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void mul_i8(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            signed char a = v_i8[i], b = v_i8[j], r;
            if (__builtin_mul_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + (unsigned long long)(long long)r;
        }
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            signed char r;
            m += __builtin_mul_overflow(v_i8[i], v_i8[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void add_u16(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned short a = v_u16[i], b = v_u16[j], r;
            if (__builtin_add_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + r;
        }
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned short r;
            m += __builtin_add_overflow(v_u16[i], v_u16[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void sub_u16(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned short a = v_u16[i], b = v_u16[j], r;
            if (__builtin_sub_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + r;
        }
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned short r;
            m += __builtin_sub_overflow(v_u16[i], v_u16[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void mul_u16(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned short a = v_u16[i], b = v_u16[j], r;
            if (__builtin_mul_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + r;
        }
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned short r;
            m += __builtin_mul_overflow(v_u16[i], v_u16[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void add_i16(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            short a = v_i16[i], b = v_i16[j], r;
            if (__builtin_add_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + (unsigned long long)(long long)r;
        }
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            short r;
            m += __builtin_add_overflow(v_i16[i], v_i16[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void sub_i16(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            short a = v_i16[i], b = v_i16[j], r;
            if (__builtin_sub_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + (unsigned long long)(long long)r;
        }
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            short r;
            m += __builtin_sub_overflow(v_i16[i], v_i16[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void mul_i16(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            short a = v_i16[i], b = v_i16[j], r;
            if (__builtin_mul_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + (unsigned long long)(long long)r;
        }
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            short r;
            m += __builtin_mul_overflow(v_i16[i], v_i16[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void add_u32(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned a = v_u32[i], b = v_u32[j], r;
            if (__builtin_add_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + r;
        }
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned r;
            m += __builtin_add_overflow(v_u32[i], v_u32[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void sub_u32(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned a = v_u32[i], b = v_u32[j], r;
            if (__builtin_sub_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + r;
        }
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned r;
            m += __builtin_sub_overflow(v_u32[i], v_u32[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void mul_u32(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned a = v_u32[i], b = v_u32[j], r;
            if (__builtin_mul_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + r;
        }
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned r;
            m += __builtin_mul_overflow(v_u32[i], v_u32[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void add_i32(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            int a = v_i32[i], b = v_i32[j], r;
            if (__builtin_add_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + (unsigned long long)(long long)r;
        }
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            int r;
            m += __builtin_add_overflow(v_i32[i], v_i32[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void sub_i32(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            int a = v_i32[i], b = v_i32[j], r;
            if (__builtin_sub_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + (unsigned long long)(long long)r;
        }
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            int r;
            m += __builtin_sub_overflow(v_i32[i], v_i32[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void mul_i32(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            int a = v_i32[i], b = v_i32[j], r;
            if (__builtin_mul_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + (unsigned long long)(long long)r;
        }
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            int r;
            m += __builtin_mul_overflow(v_i32[i], v_i32[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void add_u64(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned long long a = v_u64[i], b = v_u64[j], r;
            if (__builtin_add_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + r;
        }
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned long long r;
            m += __builtin_add_overflow(v_u64[i], v_u64[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void sub_u64(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned long long a = v_u64[i], b = v_u64[j], r;
            if (__builtin_sub_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + r;
        }
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned long long r;
            m += __builtin_sub_overflow(v_u64[i], v_u64[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void mul_u64(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned long long a = v_u64[i], b = v_u64[j], r;
            if (__builtin_mul_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + r;
        }
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            unsigned long long r;
            m += __builtin_mul_overflow(v_u64[i], v_u64[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void add_i64(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            long long a = v_i64[i], b = v_i64[j], r;
            if (__builtin_add_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + (unsigned long long)(long long)r;
        }
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            long long r;
            m += __builtin_add_overflow(v_i64[i], v_i64[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void sub_i64(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            long long a = v_i64[i], b = v_i64[j], r;
            if (__builtin_sub_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + (unsigned long long)(long long)r;
        }
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            long long r;
            m += __builtin_sub_overflow(v_i64[i], v_i64[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
static void mul_i64(void) {
    unsigned long long h = 0, n = 0, m = 0;
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            long long a = v_i64[i], b = v_i64[j], r;
            if (__builtin_mul_overflow(a, b, &r)) {
                n++;
                h = h * 31 + 7;
                continue;
            }
            h = h * 31 + (unsigned long long)(long long)r;
        }
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 25; j++) {
            long long r;
            m += __builtin_mul_overflow(v_i64[i], v_i64[j], &r);
        }
    print_u64(n);
    print_u64(h);
    print_u64(m);
}
int main(void) {
    add_u8();
    sub_u8();
    mul_u8();
    add_i8();
    sub_i8();
    mul_i8();
    add_u16();
    sub_u16();
    mul_u16();
    add_i16();
    sub_i16();
    mul_i16();
    add_u32();
    sub_u32();
    mul_u32();
    add_i32();
    sub_i32();
    mul_i32();
    add_u64();
    sub_u64();
    mul_u64();
    add_i64();
    sub_i64();
    mul_i64();
    return 0;
}
/* expect: 39 */
/* expect: 14760798695407207673 */
/* expect: 39 */
/* expect: 66 */
/* expect: 10625826488230510042 */
/* expect: 66 */
/* expect: 77 */
/* expect: 7930997519587918666 */
/* expect: 77 */
/* expect: 88 */
/* expect: 12377986013288077342 */
/* expect: 88 */
/* expect: 88 */
/* expect: 7683311316591818752 */
/* expect: 88 */
/* expect: 405 */
/* expect: 2948736826052273918 */
/* expect: 405 */
/* expect: 39 */
/* expect: 3862347083562843641 */
/* expect: 39 */
/* expect: 66 */
/* expect: 10256516613640806874 */
/* expect: 66 */
/* expect: 77 */
/* expect: 742071294359359722 */
/* expect: 77 */
/* expect: 88 */
/* expect: 5580892345369380158 */
/* expect: 88 */
/* expect: 88 */
/* expect: 4104404999633172736 */
/* expect: 88 */
/* expect: 405 */
/* expect: 4234314348334409438 */
/* expect: 405 */
/* expect: 39 */
/* expect: 14584938140633791993 */
/* expect: 39 */
/* expect: 66 */
/* expect: 4737110629686714842 */
/* expect: 66 */
/* expect: 77 */
/* expect: 13458277334614403818 */
/* expect: 77 */
/* expect: 88 */
/* expect: 10257536752726996798 */
/* expect: 88 */
/* expect: 88 */
/* expect: 13585529952386316544 */
/* expect: 88 */
/* expect: 397 */
/* expect: 13011639258650540566 */
/* expect: 397 */
/* expect: 39 */
/* expect: 4673379382004678137 */
/* expect: 39 */
/* expect: 66 */
/* expect: 17432413694133019098 */
/* expect: 66 */
/* expect: 77 */
/* expect: 17724156275395914474 */
/* expect: 77 */
/* expect: 88 */
/* expect: 3194332595878799166 */
/* expect: 88 */
/* expect: 88 */
/* expect: 17824986912529381632 */
/* expect: 88 */
/* expect: 397 */
/* expect: 12807630375680465430 */
/* expect: 397 */
