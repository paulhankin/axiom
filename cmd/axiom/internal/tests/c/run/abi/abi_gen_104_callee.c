/* abi_gen_104_callee.c - written by gen_c_abi.py 104 8: the functions
   that abi_gen_104.c calls, which check their arguments. */
#include "abi_gen_104.h"

char abi_anchor[8];
int abi_failures;
int abi_first_test = -1, abi_first_what;

void abi_fail(int test, int what) {
    if (abi_failures++ == 0) {
        abi_first_test = test;
        abi_first_what = what;
    }
}

struct R0 abi_f0(void) {
    struct R0 r; fill_R0(&r, 99u);
    return r;
}

void abi_f1(long long a0, long long a1) {
    if (!(a0 == (long long)abi_mix(100u, 0u))) abi_fail(1, 0);
    if (!(a1 == (long long)abi_mix(101u, 0u))) abi_fail(1, 1);
}

static void abi_v2(va_list ap, int from) {
    if (from <= 0) {
        union R16 v = va_arg(ap, union R16);
        if (!(check_R16(&v, 250u))) abi_fail(2, 200);
    }
    if (from <= 1) {
        __int128 v = va_arg(ap, __int128);
        if (!(v == (__int128)((unsigned __int128)abi_mix(251u, 0u) ^ ((unsigned __int128)abi_mix(251u, 0u + 2000u) << 64)))) abi_fail(2, 201);
    }
}
struct R1 abi_f2(float a0, float a1, struct R2 a2, struct R4 a3, struct R5 a4, struct R6 a5, union R12 a6, struct R13 a7, struct R14 a8, _BitInt(128) a9, signed char a10, double a11, ...) {
    if (!(a0 == (float)((double)(long long)(abi_mix(200u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(2, 0);
    if (!(a1 == (float)((double)(long long)(abi_mix(201u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(2, 1);
    if (!(check_R2(&a2, 202u))) abi_fail(2, 2);
    if (!(check_R4(&a3, 203u))) abi_fail(2, 3);
    if (!(check_R5(&a4, 204u))) abi_fail(2, 4);
    if (!(check_R6(&a5, 205u))) abi_fail(2, 5);
    if (!(check_R12(&a6, 206u))) abi_fail(2, 6);
    if (!(check_R13(&a7, 207u))) abi_fail(2, 7);
    if (!(check_R14(&a8, 208u))) abi_fail(2, 8);
    if (!(a9 == (_BitInt(128))((unsigned _BitInt(128))abi_mix(209u, 0u) ^ ((unsigned _BitInt(128))abi_mix(209u, 0u + 2000u) << 64)))) abi_fail(2, 9);
    if (!(a10 == (signed char)((long long)(abi_mix(210u, 0u) % 256u) - 128))) abi_fail(2, 10);
    if (!(a11 == (double)((double)(long long)(abi_mix(211u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(2, 11);
    va_list ap, aq, ar;
    va_start(ap, a11);
    va_copy(aq, ap);
    {
        union R16 v = va_arg(ap, union R16);
        if (!(check_R16(&v, 250u))) abi_fail(2, 100);
    }
    {
        __int128 v = va_arg(ap, __int128);
        if (!(v == (__int128)((unsigned __int128)abi_mix(251u, 0u) ^ ((unsigned __int128)abi_mix(251u, 0u + 2000u) << 64)))) abi_fail(2, 101);
    }
    va_end(ap);
    va_copy(ar, aq);
    abi_v2(aq, 0);
    va_end(aq);
    abi_v2(ar, 0);
    va_end(ar);
    struct R1 r; fill_R1(&r, 299u);
    return r;
}

static void abi_v3(va_list ap, int from) {
    if (from <= 0) {
        unsigned _BitInt(256) v = va_arg(ap, unsigned _BitInt(256));
        if (!(v == (unsigned _BitInt(256))((unsigned _BitInt(256))abi_mix(350u, 0u) ^ ((unsigned _BitInt(256))abi_mix(350u, 0u + 2000u) << 64) ^ ((unsigned _BitInt(256))abi_mix(350u, 0u + 3000u) << 192)))) abi_fail(3, 200);
    }
    if (from <= 1) {
        char * v = va_arg(ap, char *);
        if (!(v == (abi_anchor + (abi_mix(351u, 0u) % 8u)))) abi_fail(3, 201);
    }
    if (from <= 2) {
        struct R19 v = va_arg(ap, struct R19);
        if (!(check_R19(&v, 352u))) abi_fail(3, 202);
    }
    if (from <= 3) {
        long long v = va_arg(ap, long long);
        if (!(v == (long long)abi_mix(353u, 0u))) abi_fail(3, 203);
    }
    if (from <= 4) {
        struct R20 v = va_arg(ap, struct R20);
        if (!(check_R20(&v, 354u))) abi_fail(3, 204);
    }
    if (from <= 5) {
        int v = va_arg(ap, int);
        if (!(v == (int)((long long)(abi_mix(355u, 0u) % 4294967296u) - 2147483648))) abi_fail(3, 205);
    }
}
struct R18 abi_f3(unsigned long long a0, ...) {
    if (!(a0 == (unsigned long long)abi_mix(300u, 0u))) abi_fail(3, 0);
    va_list ap, aq, ar;
    va_start(ap, a0);
    {
        unsigned _BitInt(256) v = va_arg(ap, unsigned _BitInt(256));
        if (!(v == (unsigned _BitInt(256))((unsigned _BitInt(256))abi_mix(350u, 0u) ^ ((unsigned _BitInt(256))abi_mix(350u, 0u + 2000u) << 64) ^ ((unsigned _BitInt(256))abi_mix(350u, 0u + 3000u) << 192)))) abi_fail(3, 100);
    }
    {
        char * v = va_arg(ap, char *);
        if (!(v == (abi_anchor + (abi_mix(351u, 0u) % 8u)))) abi_fail(3, 101);
    }
    {
        struct R19 v = va_arg(ap, struct R19);
        if (!(check_R19(&v, 352u))) abi_fail(3, 102);
    }
    {
        long long v = va_arg(ap, long long);
        if (!(v == (long long)abi_mix(353u, 0u))) abi_fail(3, 103);
    }
    va_copy(aq, ap);
    {
        struct R20 v = va_arg(ap, struct R20);
        if (!(check_R20(&v, 354u))) abi_fail(3, 104);
    }
    {
        int v = va_arg(ap, int);
        if (!(v == (int)((long long)(abi_mix(355u, 0u) % 4294967296u) - 2147483648))) abi_fail(3, 105);
    }
    va_end(ap);
    va_copy(ar, aq);
    abi_v3(aq, 4);
    va_end(aq);
    abi_v3(ar, 4);
    va_end(ar);
    struct R18 r; fill_R18(&r, 399u);
    return r;
}

_Bool abi_f4(union R21 a0, struct R26 a1) {
    if (!(check_R21(&a0, 400u))) abi_fail(4, 0);
    if (!(check_R26(&a1, 401u))) abi_fail(4, 1);
    _Bool r = (_Bool)(abi_mix(499u, 0u) & 1u);
    return r;
}

struct R27 abi_f5(struct R28 a0) {
    if (!(check_R28(&a0, 500u))) abi_fail(5, 0);
    struct R27 r; fill_R27(&r, 599u);
    return r;
}

struct R29 abi_f6(union R30 a0, _BitInt(65) a1, struct R31 a2, struct R33 a3) {
    if (!(check_R30(&a0, 600u))) abi_fail(6, 0);
    if (!(a1 == (_BitInt(65))((unsigned _BitInt(65))abi_mix(601u, 0u) ^ ((unsigned _BitInt(65))abi_mix(601u, 0u + 2000u) << 64)))) abi_fail(6, 1);
    if (!(check_R31(&a2, 602u))) abi_fail(6, 2);
    if (!(check_R33(&a3, 603u))) abi_fail(6, 3);
    struct R29 r; fill_R29(&r, 699u);
    return r;
}

static void abi_v7(va_list ap, int from) {
    if (from <= 0) {
        struct R39 v = va_arg(ap, struct R39);
        if (!(check_R39(&v, 750u))) abi_fail(7, 200);
    }
    if (from <= 1) {
        struct R40 v = va_arg(ap, struct R40);
        if (!(check_R40(&v, 751u))) abi_fail(7, 201);
    }
    if (from <= 2) {
        union R41 v = va_arg(ap, union R41);
        if (!(check_R41(&v, 752u))) abi_fail(7, 202);
    }
}
void abi_f7(unsigned char a0, unsigned __int128 a1, struct R34 a2, struct R35 a3, struct R36 a4, struct R37 a5, unsigned long long a6, struct R38 a7, ...) {
    if (!(a0 == (unsigned char)(abi_mix(700u, 0u) % 256u))) abi_fail(7, 0);
    if (!(a1 == (unsigned __int128)((unsigned __int128)abi_mix(701u, 0u) ^ ((unsigned __int128)abi_mix(701u, 0u + 2000u) << 64)))) abi_fail(7, 1);
    if (!(check_R34(&a2, 702u))) abi_fail(7, 2);
    if (!(check_R35(&a3, 703u))) abi_fail(7, 3);
    if (!(check_R36(&a4, 704u))) abi_fail(7, 4);
    if (!(check_R37(&a5, 705u))) abi_fail(7, 5);
    if (!(a6 == (unsigned long long)abi_mix(706u, 0u))) abi_fail(7, 6);
    if (!(check_R38(&a7, 707u))) abi_fail(7, 7);
    va_list ap, aq, ar;
    va_start(ap, a7);
    {
        struct R39 v = va_arg(ap, struct R39);
        if (!(check_R39(&v, 750u))) abi_fail(7, 100);
    }
    va_copy(aq, ap);
    {
        struct R40 v = va_arg(ap, struct R40);
        if (!(check_R40(&v, 751u))) abi_fail(7, 101);
    }
    {
        union R41 v = va_arg(ap, union R41);
        if (!(check_R41(&v, 752u))) abi_fail(7, 102);
    }
    va_end(ap);
    va_copy(ar, aq);
    abi_v7(aq, 1);
    va_end(aq);
    abi_v7(ar, 1);
    va_end(ar);
}

int abi_result(void) {
    return abi_failures;
}
int abi_first(void) {
    return abi_first_test * 1000 + abi_first_what;
}
unsigned long long abi_layout(int k) {
    switch (k) {
    case 0: return sizeof(struct R0) * 1000 + _Alignof(struct R0);
    case 1: return sizeof(struct R1) * 1000 + _Alignof(struct R1);
    case 2: return sizeof(struct R2) * 1000 + _Alignof(struct R2);
    case 3: return sizeof(struct R3) * 1000 + _Alignof(struct R3);
    case 4: return sizeof(struct R4) * 1000 + _Alignof(struct R4);
    case 5: return sizeof(struct R5) * 1000 + _Alignof(struct R5);
    case 6: return sizeof(struct R6) * 1000 + _Alignof(struct R6);
    case 7: return sizeof(struct R7) * 1000 + _Alignof(struct R7);
    case 8: return sizeof(union R8) * 1000 + _Alignof(union R8);
    case 9: return sizeof(struct R9) * 1000 + _Alignof(struct R9);
    case 10: return sizeof(struct R10) * 1000 + _Alignof(struct R10);
    case 11: return sizeof(struct R11) * 1000 + _Alignof(struct R11);
    case 12: return sizeof(union R12) * 1000 + _Alignof(union R12);
    case 13: return sizeof(struct R13) * 1000 + _Alignof(struct R13);
    case 14: return sizeof(struct R14) * 1000 + _Alignof(struct R14);
    case 15: return sizeof(struct R15) * 1000 + _Alignof(struct R15);
    case 16: return sizeof(union R16) * 1000 + _Alignof(union R16);
    case 17: return sizeof(struct R17) * 1000 + _Alignof(struct R17);
    case 18: return sizeof(struct R18) * 1000 + _Alignof(struct R18);
    case 19: return sizeof(struct R19) * 1000 + _Alignof(struct R19);
    case 20: return sizeof(struct R20) * 1000 + _Alignof(struct R20);
    case 21: return sizeof(union R21) * 1000 + _Alignof(union R21);
    case 22: return sizeof(struct R22) * 1000 + _Alignof(struct R22);
    case 23: return sizeof(struct R23) * 1000 + _Alignof(struct R23);
    case 24: return sizeof(struct R24) * 1000 + _Alignof(struct R24);
    case 25: return sizeof(struct R25) * 1000 + _Alignof(struct R25);
    case 26: return sizeof(struct R26) * 1000 + _Alignof(struct R26);
    case 27: return sizeof(struct R27) * 1000 + _Alignof(struct R27);
    case 28: return sizeof(struct R28) * 1000 + _Alignof(struct R28);
    case 29: return sizeof(struct R29) * 1000 + _Alignof(struct R29);
    case 30: return sizeof(union R30) * 1000 + _Alignof(union R30);
    case 31: return sizeof(struct R31) * 1000 + _Alignof(struct R31);
    case 32: return sizeof(struct R32) * 1000 + _Alignof(struct R32);
    case 33: return sizeof(struct R33) * 1000 + _Alignof(struct R33);
    case 34: return sizeof(struct R34) * 1000 + _Alignof(struct R34);
    case 35: return sizeof(struct R35) * 1000 + _Alignof(struct R35);
    case 36: return sizeof(struct R36) * 1000 + _Alignof(struct R36);
    case 37: return sizeof(struct R37) * 1000 + _Alignof(struct R37);
    case 38: return sizeof(struct R38) * 1000 + _Alignof(struct R38);
    case 39: return sizeof(struct R39) * 1000 + _Alignof(struct R39);
    case 40: return sizeof(struct R40) * 1000 + _Alignof(struct R40);
    case 41: return sizeof(union R41) * 1000 + _Alignof(union R41);
    }
    return 0;
}
