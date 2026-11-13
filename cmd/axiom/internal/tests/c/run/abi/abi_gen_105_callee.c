/* abi_gen_105_callee.c - written by gen_c_abi.py 105 8: the functions
   that abi_gen_105.c calls, which check their arguments. */
#include "abi_gen_105.h"

char abi_anchor[8];
int abi_failures;
int abi_first_test = -1, abi_first_what;

void abi_fail(int test, int what) {
    if (abi_failures++ == 0) {
        abi_first_test = test;
        abi_first_what = what;
    }
}

static void abi_v0(va_list ap, int from) {
    if (from <= 0) {
        struct R6 v = va_arg(ap, struct R6);
        if (!(check_R6(&v, 50u))) abi_fail(0, 200);
    }
}
struct R0 abi_f0(struct R2 a0, ...) {
    if (!(check_R2(&a0, 0u))) abi_fail(0, 0);
    va_list ap, aq, ar;
    va_start(ap, a0);
    va_copy(aq, ap);
    {
        struct R6 v = va_arg(ap, struct R6);
        if (!(check_R6(&v, 50u))) abi_fail(0, 100);
    }
    va_end(ap);
    va_copy(ar, aq);
    abi_v0(aq, 0);
    va_end(aq);
    abi_v0(ar, 0);
    va_end(ar);
    struct R0 r; fill_R0(&r, 99u);
    return r;
}

static void abi_v1(va_list ap, int from) {
    if (from <= 0) {
        struct R20 v = va_arg(ap, struct R20);
        if (!(check_R20(&v, 150u))) abi_fail(1, 200);
    }
    if (from <= 1) {
        struct R22 v = va_arg(ap, struct R22);
        if (!(check_R22(&v, 151u))) abi_fail(1, 201);
    }
}
signed char abi_f1(int a0, float a1, struct R7 a2, _Decimal128 a3, union R9 a4, struct R10 a5, struct R12 a6, struct R13 a7, struct R17 a8, unsigned long long a9, struct R19 a10, _Decimal32 a11, ...) {
    if (!(a0 == (int)((long long)(abi_mix(100u, 0u) % 4294967296u) - 2147483648))) abi_fail(1, 0);
    if (!(a1 == (float)((double)(long long)(abi_mix(101u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(1, 1);
    if (!(check_R7(&a2, 102u))) abi_fail(1, 2);
    if (!(a3 == (_Decimal128)((double)(long long)(abi_mix(103u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(1, 3);
    if (!(check_R9(&a4, 104u))) abi_fail(1, 4);
    if (!(check_R10(&a5, 105u))) abi_fail(1, 5);
    if (!(check_R12(&a6, 106u))) abi_fail(1, 6);
    if (!(check_R13(&a7, 107u))) abi_fail(1, 7);
    if (!(check_R17(&a8, 108u))) abi_fail(1, 8);
    if (!(a9 == (unsigned long long)abi_mix(109u, 0u))) abi_fail(1, 9);
    if (!(check_R19(&a10, 110u))) abi_fail(1, 10);
    if (!(a11 == (_Decimal32)((double)(long long)(abi_mix(111u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(1, 11);
    va_list ap, aq, ar;
    va_start(ap, a11);
    va_copy(aq, ap);
    {
        struct R20 v = va_arg(ap, struct R20);
        if (!(check_R20(&v, 150u))) abi_fail(1, 100);
    }
    {
        struct R22 v = va_arg(ap, struct R22);
        if (!(check_R22(&v, 151u))) abi_fail(1, 101);
    }
    va_end(ap);
    va_copy(ar, aq);
    abi_v1(aq, 0);
    va_end(aq);
    abi_v1(ar, 0);
    va_end(ar);
    signed char r = (signed char)((long long)(abi_mix(199u, 0u) % 256u) - 128);
    return r;
}

struct R23 abi_f2(int a0, long long a1, int a2, long long a3, int a4, float a5, float a6, struct R25 a7, struct R26 a8, struct R27 a9, struct R28 a10, struct R30 a11, struct R34 a12, struct R35 a13, struct R36 a14, struct R37 a15, _Decimal32 a16) {
    if (!(a0 == (int)((long long)(abi_mix(200u, 0u) % 4294967296u) - 2147483648))) abi_fail(2, 0);
    if (!(a1 == (long long)abi_mix(201u, 0u))) abi_fail(2, 1);
    if (!(a2 == (int)((long long)(abi_mix(202u, 0u) % 4294967296u) - 2147483648))) abi_fail(2, 2);
    if (!(a3 == (long long)abi_mix(203u, 0u))) abi_fail(2, 3);
    if (!(a4 == (int)((long long)(abi_mix(204u, 0u) % 4294967296u) - 2147483648))) abi_fail(2, 4);
    if (!(a5 == (float)((double)(long long)(abi_mix(205u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(2, 5);
    if (!(a6 == (float)((double)(long long)(abi_mix(206u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(2, 6);
    if (!(check_R25(&a7, 207u))) abi_fail(2, 7);
    if (!(check_R26(&a8, 208u))) abi_fail(2, 8);
    if (!(check_R27(&a9, 209u))) abi_fail(2, 9);
    if (!(check_R28(&a10, 210u))) abi_fail(2, 10);
    if (!(check_R30(&a11, 211u))) abi_fail(2, 11);
    if (!(check_R34(&a12, 212u))) abi_fail(2, 12);
    if (!(check_R35(&a13, 213u))) abi_fail(2, 13);
    if (!(check_R36(&a14, 214u))) abi_fail(2, 14);
    if (!(check_R37(&a15, 215u))) abi_fail(2, 15);
    if (!(a16 == (_Decimal32)((double)(long long)(abi_mix(216u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(2, 16);
    struct R23 r; fill_R23(&r, 299u);
    return r;
}

union R38 abi_f3(struct R39 a0, struct R40 a1, struct R41 a2, _Bool a3) {
    if (!(check_R39(&a0, 300u))) abi_fail(3, 0);
    if (!(check_R40(&a1, 301u))) abi_fail(3, 1);
    if (!(check_R41(&a2, 302u))) abi_fail(3, 2);
    if (!(a3 == (_Bool)(abi_mix(303u, 0u) & 1u))) abi_fail(3, 3);
    union R38 r; fill_R38(&r, 399u);
    return r;
}

struct R42 abi_f4(int a0, float a1, long long a2, int a3, int a4, float a5, int a6, int a7, struct R43 a8, struct R46 a9) {
    if (!(a0 == (int)((long long)(abi_mix(400u, 0u) % 4294967296u) - 2147483648))) abi_fail(4, 0);
    if (!(a1 == (float)((double)(long long)(abi_mix(401u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(4, 1);
    if (!(a2 == (long long)abi_mix(402u, 0u))) abi_fail(4, 2);
    if (!(a3 == (int)((long long)(abi_mix(403u, 0u) % 4294967296u) - 2147483648))) abi_fail(4, 3);
    if (!(a4 == (int)((long long)(abi_mix(404u, 0u) % 4294967296u) - 2147483648))) abi_fail(4, 4);
    if (!(a5 == (float)((double)(long long)(abi_mix(405u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(4, 5);
    if (!(a6 == (int)((long long)(abi_mix(406u, 0u) % 4294967296u) - 2147483648))) abi_fail(4, 6);
    if (!(a7 == (int)((long long)(abi_mix(407u, 0u) % 4294967296u) - 2147483648))) abi_fail(4, 7);
    if (!(check_R43(&a8, 408u))) abi_fail(4, 8);
    if (!(check_R46(&a9, 409u))) abi_fail(4, 9);
    struct R42 r; fill_R42(&r, 499u);
    return r;
}

union R47 abi_f5(_Decimal32 a0, char * a1, unsigned short a2, struct R49 a3, struct R50 a4, struct R54 a5, double a6, struct R55 a7, struct R56 a8, struct R61 a9) {
    if (!(a0 == (_Decimal32)((double)(long long)(abi_mix(500u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(5, 0);
    if (!(a1 == (abi_anchor + (abi_mix(501u, 0u) % 8u)))) abi_fail(5, 1);
    if (!(a2 == (unsigned short)(abi_mix(502u, 0u) % 65536u))) abi_fail(5, 2);
    if (!(check_R49(&a3, 503u))) abi_fail(5, 3);
    if (!(check_R50(&a4, 504u))) abi_fail(5, 4);
    if (!(check_R54(&a5, 505u))) abi_fail(5, 5);
    if (!(a6 == (double)((double)(long long)(abi_mix(506u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(5, 6);
    if (!(check_R55(&a7, 507u))) abi_fail(5, 7);
    if (!(check_R56(&a8, 508u))) abi_fail(5, 8);
    if (!(check_R61(&a9, 509u))) abi_fail(5, 9);
    union R47 r; fill_R47(&r, 599u);
    return r;
}

static void abi_v6(va_list ap, int from) {
    if (from <= 0) {
        _Decimal128 v = va_arg(ap, _Decimal128);
        if (!(v == (_Decimal128)((double)(long long)(abi_mix(650u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(6, 200);
    }
    if (from <= 1) {
        _Decimal32 v = va_arg(ap, _Decimal32);
        if (!(v == (_Decimal32)((double)(long long)(abi_mix(651u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(6, 201);
    }
}
struct R62 abi_f6(struct R64 a0, struct R66 a1, struct R67 a2, unsigned long long a3, ...) {
    if (!(check_R64(&a0, 600u))) abi_fail(6, 0);
    if (!(check_R66(&a1, 601u))) abi_fail(6, 1);
    if (!(check_R67(&a2, 602u))) abi_fail(6, 2);
    if (!(a3 == (unsigned long long)abi_mix(603u, 0u))) abi_fail(6, 3);
    va_list ap, aq, ar;
    va_start(ap, a3);
    va_copy(aq, ap);
    {
        _Decimal128 v = va_arg(ap, _Decimal128);
        if (!(v == (_Decimal128)((double)(long long)(abi_mix(650u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(6, 100);
    }
    {
        _Decimal32 v = va_arg(ap, _Decimal32);
        if (!(v == (_Decimal32)((double)(long long)(abi_mix(651u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(6, 101);
    }
    va_end(ap);
    va_copy(ar, aq);
    abi_v6(aq, 0);
    va_end(aq);
    abi_v6(ar, 0);
    va_end(ar);
    struct R62 r; fill_R62(&r, 699u);
    return r;
}

static void abi_v7(va_list ap, int from) {
    if (from <= 0) {
        struct R72 v = va_arg(ap, struct R72);
        if (!(check_R72(&v, 750u))) abi_fail(7, 200);
    }
    if (from <= 1) {
        struct R73 v = va_arg(ap, struct R73);
        if (!(check_R73(&v, 751u))) abi_fail(7, 201);
    }
    if (from <= 2) {
        int v = va_arg(ap, int);
        if (!(v == (int)((long long)(abi_mix(752u, 0u) % 4294967296u) - 2147483648))) abi_fail(7, 202);
    }
    if (from <= 3) {
        struct R74 v = va_arg(ap, struct R74);
        if (!(check_R74(&v, 753u))) abi_fail(7, 203);
    }
}
void abi_f7(struct R68 a0, struct R69 a1, ...) {
    if (!(check_R68(&a0, 700u))) abi_fail(7, 0);
    if (!(check_R69(&a1, 701u))) abi_fail(7, 1);
    va_list ap, aq, ar;
    va_start(ap, a1);
    {
        struct R72 v = va_arg(ap, struct R72);
        if (!(check_R72(&v, 750u))) abi_fail(7, 100);
    }
    {
        struct R73 v = va_arg(ap, struct R73);
        if (!(check_R73(&v, 751u))) abi_fail(7, 101);
    }
    va_copy(aq, ap);
    {
        int v = va_arg(ap, int);
        if (!(v == (int)((long long)(abi_mix(752u, 0u) % 4294967296u) - 2147483648))) abi_fail(7, 102);
    }
    {
        struct R74 v = va_arg(ap, struct R74);
        if (!(check_R74(&v, 753u))) abi_fail(7, 103);
    }
    va_end(ap);
    va_copy(ar, aq);
    abi_v7(aq, 2);
    va_end(aq);
    abi_v7(ar, 2);
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
    case 4: return sizeof(union R4) * 1000 + _Alignof(union R4);
    case 5: return sizeof(struct R5) * 1000 + _Alignof(struct R5);
    case 6: return sizeof(struct R6) * 1000 + _Alignof(struct R6);
    case 7: return sizeof(struct R7) * 1000 + _Alignof(struct R7);
    case 8: return sizeof(struct R8) * 1000 + _Alignof(struct R8);
    case 9: return sizeof(union R9) * 1000 + _Alignof(union R9);
    case 10: return sizeof(struct R10) * 1000 + _Alignof(struct R10);
    case 11: return sizeof(struct R11) * 1000 + _Alignof(struct R11);
    case 12: return sizeof(struct R12) * 1000 + _Alignof(struct R12);
    case 13: return sizeof(struct R13) * 1000 + _Alignof(struct R13);
    case 14: return sizeof(struct R14) * 1000 + _Alignof(struct R14);
    case 15: return sizeof(struct R15) * 1000 + _Alignof(struct R15);
    case 16: return sizeof(struct R16) * 1000 + _Alignof(struct R16);
    case 17: return sizeof(struct R17) * 1000 + _Alignof(struct R17);
    case 18: return sizeof(union R18) * 1000 + _Alignof(union R18);
    case 19: return sizeof(struct R19) * 1000 + _Alignof(struct R19);
    case 20: return sizeof(struct R20) * 1000 + _Alignof(struct R20);
    case 21: return sizeof(struct R21) * 1000 + _Alignof(struct R21);
    case 22: return sizeof(struct R22) * 1000 + _Alignof(struct R22);
    case 23: return sizeof(struct R23) * 1000 + _Alignof(struct R23);
    case 24: return sizeof(struct R24) * 1000 + _Alignof(struct R24);
    case 25: return sizeof(struct R25) * 1000 + _Alignof(struct R25);
    case 26: return sizeof(struct R26) * 1000 + _Alignof(struct R26);
    case 27: return sizeof(struct R27) * 1000 + _Alignof(struct R27);
    case 28: return sizeof(struct R28) * 1000 + _Alignof(struct R28);
    case 29: return sizeof(struct R29) * 1000 + _Alignof(struct R29);
    case 30: return sizeof(struct R30) * 1000 + _Alignof(struct R30);
    case 31: return sizeof(struct R31) * 1000 + _Alignof(struct R31);
    case 32: return sizeof(struct R32) * 1000 + _Alignof(struct R32);
    case 33: return sizeof(union R33) * 1000 + _Alignof(union R33);
    case 34: return sizeof(struct R34) * 1000 + _Alignof(struct R34);
    case 35: return sizeof(struct R35) * 1000 + _Alignof(struct R35);
    case 36: return sizeof(struct R36) * 1000 + _Alignof(struct R36);
    case 37: return sizeof(struct R37) * 1000 + _Alignof(struct R37);
    case 38: return sizeof(union R38) * 1000 + _Alignof(union R38);
    case 39: return sizeof(struct R39) * 1000 + _Alignof(struct R39);
    case 40: return sizeof(struct R40) * 1000 + _Alignof(struct R40);
    case 41: return sizeof(struct R41) * 1000 + _Alignof(struct R41);
    case 42: return sizeof(struct R42) * 1000 + _Alignof(struct R42);
    case 43: return sizeof(struct R43) * 1000 + _Alignof(struct R43);
    case 44: return sizeof(struct R44) * 1000 + _Alignof(struct R44);
    case 45: return sizeof(struct R45) * 1000 + _Alignof(struct R45);
    case 46: return sizeof(struct R46) * 1000 + _Alignof(struct R46);
    case 47: return sizeof(union R47) * 1000 + _Alignof(union R47);
    case 48: return sizeof(struct R48) * 1000 + _Alignof(struct R48);
    case 49: return sizeof(struct R49) * 1000 + _Alignof(struct R49);
    case 50: return sizeof(struct R50) * 1000 + _Alignof(struct R50);
    case 51: return sizeof(struct R51) * 1000 + _Alignof(struct R51);
    case 52: return sizeof(struct R52) * 1000 + _Alignof(struct R52);
    case 53: return sizeof(struct R53) * 1000 + _Alignof(struct R53);
    case 54: return sizeof(struct R54) * 1000 + _Alignof(struct R54);
    case 55: return sizeof(struct R55) * 1000 + _Alignof(struct R55);
    case 56: return sizeof(struct R56) * 1000 + _Alignof(struct R56);
    case 57: return sizeof(struct R57) * 1000 + _Alignof(struct R57);
    case 58: return sizeof(struct R58) * 1000 + _Alignof(struct R58);
    case 59: return sizeof(struct R59) * 1000 + _Alignof(struct R59);
    case 60: return sizeof(union R60) * 1000 + _Alignof(union R60);
    case 61: return sizeof(struct R61) * 1000 + _Alignof(struct R61);
    case 62: return sizeof(struct R62) * 1000 + _Alignof(struct R62);
    case 63: return sizeof(struct R63) * 1000 + _Alignof(struct R63);
    case 64: return sizeof(struct R64) * 1000 + _Alignof(struct R64);
    case 65: return sizeof(struct R65) * 1000 + _Alignof(struct R65);
    case 66: return sizeof(struct R66) * 1000 + _Alignof(struct R66);
    case 67: return sizeof(struct R67) * 1000 + _Alignof(struct R67);
    case 68: return sizeof(struct R68) * 1000 + _Alignof(struct R68);
    case 69: return sizeof(struct R69) * 1000 + _Alignof(struct R69);
    case 70: return sizeof(struct R70) * 1000 + _Alignof(struct R70);
    case 71: return sizeof(struct R71) * 1000 + _Alignof(struct R71);
    case 72: return sizeof(struct R72) * 1000 + _Alignof(struct R72);
    case 73: return sizeof(struct R73) * 1000 + _Alignof(struct R73);
    case 74: return sizeof(struct R74) * 1000 + _Alignof(struct R74);
    }
    return 0;
}
