/* abi_gen_101_callee.c - written by gen_c_abi.py 101 8: the functions
   that abi_gen_101.c calls, which check their arguments. */
#include "abi_gen_101.h"

char abi_anchor[8];
int abi_failures;
int abi_first_test = -1, abi_first_what;

void abi_fail(int test, int what) {
    if (abi_failures++ == 0) {
        abi_first_test = test;
        abi_first_what = what;
    }
}

float abi_f0(float a0, unsigned short a1, struct R0 a2, struct R1 a3, unsigned a4, signed char a5, struct R2 a6, double a7, struct R3 a8, struct R4 a9) {
    if (!(a0 == (float)((double)(long long)(abi_mix(0u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(0, 0);
    if (!(a1 == (unsigned short)(abi_mix(1u, 0u) % 65536u))) abi_fail(0, 1);
    if (!(check_R0(&a2, 2u))) abi_fail(0, 2);
    if (!(check_R1(&a3, 3u))) abi_fail(0, 3);
    if (!(a4 == (unsigned)(abi_mix(4u, 0u) % 4294967296u))) abi_fail(0, 4);
    if (!(a5 == (signed char)((long long)(abi_mix(5u, 0u) % 256u) - 128))) abi_fail(0, 5);
    if (!(check_R2(&a6, 6u))) abi_fail(0, 6);
    if (!(a7 == (double)((double)(long long)(abi_mix(7u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(0, 7);
    if (!(check_R3(&a8, 8u))) abi_fail(0, 8);
    if (!(check_R4(&a9, 9u))) abi_fail(0, 9);
    float r = (float)((double)(long long)(abi_mix(99u, 0u) % 4001u) / 8.0 - 250.0);
    return r;
}

_Bool abi_f1(struct R5 a0, struct R6 a1, int a2, int a3, struct R9 a4) {
    if (!(check_R5(&a0, 100u))) abi_fail(1, 0);
    if (!(check_R6(&a1, 101u))) abi_fail(1, 1);
    if (!(a2 == (int)((long long)(abi_mix(102u, 0u) % 4294967296u) - 2147483648))) abi_fail(1, 2);
    if (!(a3 == (int)((long long)(abi_mix(103u, 0u) % 4294967296u) - 2147483648))) abi_fail(1, 3);
    if (!(check_R9(&a4, 104u))) abi_fail(1, 4);
    _Bool r = (_Bool)(abi_mix(199u, 0u) & 1u);
    return r;
}

struct R10 abi_f2(union R13 a0, struct R14 a1, struct R15 a2, struct R17 a3, float a4) {
    if (!(check_R13(&a0, 200u))) abi_fail(2, 0);
    if (!(check_R14(&a1, 201u))) abi_fail(2, 1);
    if (!(check_R15(&a2, 202u))) abi_fail(2, 2);
    if (!(check_R17(&a3, 203u))) abi_fail(2, 3);
    if (!(a4 == (float)((double)(long long)(abi_mix(204u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(2, 4);
    struct R10 r; fill_R10(&r, 299u);
    return r;
}

struct R18 abi_f3(struct R19 a0, struct R20 a1, struct R22 a2, struct R24 a3, struct R27 a4) {
    if (!(check_R19(&a0, 300u))) abi_fail(3, 0);
    if (!(check_R20(&a1, 301u))) abi_fail(3, 1);
    if (!(check_R22(&a2, 302u))) abi_fail(3, 2);
    if (!(check_R24(&a3, 303u))) abi_fail(3, 3);
    if (!(check_R27(&a4, 304u))) abi_fail(3, 4);
    struct R18 r; fill_R18(&r, 399u);
    return r;
}

static void abi_v4(va_list ap, int from) {
    if (from <= 0) {
        double v = va_arg(ap, double);
        if (!(v == (double)((double)(long long)(abi_mix(450u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(4, 200);
    }
    if (from <= 1) {
        unsigned long long v = va_arg(ap, unsigned long long);
        if (!(v == (unsigned long long)abi_mix(451u, 0u))) abi_fail(4, 201);
    }
    if (from <= 2) {
        unsigned long long v = va_arg(ap, unsigned long long);
        if (!(v == (unsigned long long)abi_mix(452u, 0u))) abi_fail(4, 202);
    }
    if (from <= 3) {
        int v = va_arg(ap, int);
        if (!(v == (int)((long long)(abi_mix(453u, 0u) % 4294967296u) - 2147483648))) abi_fail(4, 203);
    }
}
struct R31 abi_f4(struct R32 a0, struct R34 a1, struct R39 a2, struct R40 a3, ...) {
    if (!(check_R32(&a0, 400u))) abi_fail(4, 0);
    if (!(check_R34(&a1, 401u))) abi_fail(4, 1);
    if (!(check_R39(&a2, 402u))) abi_fail(4, 2);
    if (!(check_R40(&a3, 403u))) abi_fail(4, 3);
    va_list ap, aq, ar;
    va_start(ap, a3);
    va_copy(aq, ap);
    {
        double v = va_arg(ap, double);
        if (!(v == (double)((double)(long long)(abi_mix(450u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(4, 100);
    }
    {
        unsigned long long v = va_arg(ap, unsigned long long);
        if (!(v == (unsigned long long)abi_mix(451u, 0u))) abi_fail(4, 101);
    }
    {
        unsigned long long v = va_arg(ap, unsigned long long);
        if (!(v == (unsigned long long)abi_mix(452u, 0u))) abi_fail(4, 102);
    }
    {
        int v = va_arg(ap, int);
        if (!(v == (int)((long long)(abi_mix(453u, 0u) % 4294967296u) - 2147483648))) abi_fail(4, 103);
    }
    va_end(ap);
    va_copy(ar, aq);
    abi_v4(aq, 0);
    va_end(aq);
    abi_v4(ar, 0);
    va_end(ar);
    struct R31 r; fill_R31(&r, 499u);
    return r;
}

void abi_f5(union R41 a0, unsigned char a1, struct R43 a2) {
    if (!(check_R41(&a0, 500u))) abi_fail(5, 0);
    if (!(a1 == (unsigned char)(abi_mix(501u, 0u) % 256u))) abi_fail(5, 1);
    if (!(check_R43(&a2, 502u))) abi_fail(5, 2);
}

struct R44 abi_f6(union R47 a0, struct R48 a1, _Bool a2, struct R49 a3, struct R50 a4, float a5, char * a6, struct R51 a7, struct R52 a8, double a9) {
    if (!(check_R47(&a0, 600u))) abi_fail(6, 0);
    if (!(check_R48(&a1, 601u))) abi_fail(6, 1);
    if (!(a2 == (_Bool)(abi_mix(602u, 0u) & 1u))) abi_fail(6, 2);
    if (!(check_R49(&a3, 603u))) abi_fail(6, 3);
    if (!(check_R50(&a4, 604u))) abi_fail(6, 4);
    if (!(a5 == (float)((double)(long long)(abi_mix(605u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(6, 5);
    if (!(a6 == (abi_anchor + (abi_mix(606u, 0u) % 8u)))) abi_fail(6, 6);
    if (!(check_R51(&a7, 607u))) abi_fail(6, 7);
    if (!(check_R52(&a8, 608u))) abi_fail(6, 8);
    if (!(a9 == (double)((double)(long long)(abi_mix(609u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(6, 9);
    struct R44 r; fill_R44(&r, 699u);
    return r;
}

union R53 abi_f7(double a0, struct R54 a1, struct R55 a2, unsigned a3, signed char a4, signed char a5, struct R57 a6, struct R58 a7, struct R59 a8, unsigned char a9) {
    if (!(a0 == (double)((double)(long long)(abi_mix(700u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(7, 0);
    if (!(check_R54(&a1, 701u))) abi_fail(7, 1);
    if (!(check_R55(&a2, 702u))) abi_fail(7, 2);
    if (!(a3 == (unsigned)(abi_mix(703u, 0u) % 4294967296u))) abi_fail(7, 3);
    if (!(a4 == (signed char)((long long)(abi_mix(704u, 0u) % 256u) - 128))) abi_fail(7, 4);
    if (!(a5 == (signed char)((long long)(abi_mix(705u, 0u) % 256u) - 128))) abi_fail(7, 5);
    if (!(check_R57(&a6, 706u))) abi_fail(7, 6);
    if (!(check_R58(&a7, 707u))) abi_fail(7, 7);
    if (!(check_R59(&a8, 708u))) abi_fail(7, 8);
    if (!(a9 == (unsigned char)(abi_mix(709u, 0u) % 256u))) abi_fail(7, 9);
    union R53 r; fill_R53(&r, 799u);
    return r;
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
    case 8: return sizeof(struct R8) * 1000 + _Alignof(struct R8);
    case 9: return sizeof(struct R9) * 1000 + _Alignof(struct R9);
    case 10: return sizeof(struct R10) * 1000 + _Alignof(struct R10);
    case 11: return sizeof(struct R11) * 1000 + _Alignof(struct R11);
    case 12: return sizeof(union R12) * 1000 + _Alignof(union R12);
    case 13: return sizeof(union R13) * 1000 + _Alignof(union R13);
    case 14: return sizeof(struct R14) * 1000 + _Alignof(struct R14);
    case 15: return sizeof(struct R15) * 1000 + _Alignof(struct R15);
    case 16: return sizeof(struct R16) * 1000 + _Alignof(struct R16);
    case 17: return sizeof(struct R17) * 1000 + _Alignof(struct R17);
    case 18: return sizeof(struct R18) * 1000 + _Alignof(struct R18);
    case 19: return sizeof(struct R19) * 1000 + _Alignof(struct R19);
    case 20: return sizeof(struct R20) * 1000 + _Alignof(struct R20);
    case 21: return sizeof(struct R21) * 1000 + _Alignof(struct R21);
    case 22: return sizeof(struct R22) * 1000 + _Alignof(struct R22);
    case 23: return sizeof(struct R23) * 1000 + _Alignof(struct R23);
    case 24: return sizeof(struct R24) * 1000 + _Alignof(struct R24);
    case 25: return sizeof(struct R25) * 1000 + _Alignof(struct R25);
    case 26: return sizeof(union R26) * 1000 + _Alignof(union R26);
    case 27: return sizeof(struct R27) * 1000 + _Alignof(struct R27);
    case 28: return sizeof(struct R28) * 1000 + _Alignof(struct R28);
    case 29: return sizeof(union R29) * 1000 + _Alignof(union R29);
    case 30: return sizeof(struct R30) * 1000 + _Alignof(struct R30);
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
    case 53: return sizeof(union R53) * 1000 + _Alignof(union R53);
    case 54: return sizeof(struct R54) * 1000 + _Alignof(struct R54);
    case 55: return sizeof(struct R55) * 1000 + _Alignof(struct R55);
    case 56: return sizeof(struct R56) * 1000 + _Alignof(struct R56);
    case 57: return sizeof(struct R57) * 1000 + _Alignof(struct R57);
    case 58: return sizeof(struct R58) * 1000 + _Alignof(struct R58);
    case 59: return sizeof(struct R59) * 1000 + _Alignof(struct R59);
    }
    return 0;
}
