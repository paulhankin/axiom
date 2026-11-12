/* abi_gen_102_callee.c - written by gen_c_abi.py 102 8: the functions
   that abi_gen_102.c calls, which check their arguments. */
#include "abi_gen_102.h"

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
        struct R4 v = va_arg(ap, struct R4);
        if (!(check_R4(&v, 50u))) abi_fail(0, 200);
    }
    if (from <= 1) {
        unsigned long long v = va_arg(ap, unsigned long long);
        if (!(v == (unsigned long long)abi_mix(51u, 0u))) abi_fail(0, 201);
    }
}
void abi_f0(struct R0 a0, struct R1 a1, ...) {
    if (!(check_R0(&a0, 0u))) abi_fail(0, 0);
    if (!(check_R1(&a1, 1u))) abi_fail(0, 1);
    va_list ap, aq, ar;
    va_start(ap, a1);
    va_copy(aq, ap);
    {
        struct R4 v = va_arg(ap, struct R4);
        if (!(check_R4(&v, 50u))) abi_fail(0, 100);
    }
    {
        unsigned long long v = va_arg(ap, unsigned long long);
        if (!(v == (unsigned long long)abi_mix(51u, 0u))) abi_fail(0, 101);
    }
    va_end(ap);
    va_copy(ar, aq);
    abi_v0(aq, 0);
    va_end(aq);
    abi_v0(ar, 0);
    va_end(ar);
}

void abi_f1(double a0, int a1, int a2, double a3, int a4, int a5, struct R6 a6, unsigned long long a7, unsigned long long a8, struct R7 a9, struct R8 a10, struct R9 a11, struct R11 a12) {
    if (!(a0 == (double)((double)(long long)(abi_mix(100u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(1, 0);
    if (!(a1 == (int)((long long)(abi_mix(101u, 0u) % 4294967296u) - 2147483648))) abi_fail(1, 1);
    if (!(a2 == (int)((long long)(abi_mix(102u, 0u) % 4294967296u) - 2147483648))) abi_fail(1, 2);
    if (!(a3 == (double)((double)(long long)(abi_mix(103u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(1, 3);
    if (!(a4 == (int)((long long)(abi_mix(104u, 0u) % 4294967296u) - 2147483648))) abi_fail(1, 4);
    if (!(a5 == (int)((long long)(abi_mix(105u, 0u) % 4294967296u) - 2147483648))) abi_fail(1, 5);
    if (!(check_R6(&a6, 106u))) abi_fail(1, 6);
    if (!(a7 == (unsigned long long)abi_mix(107u, 0u))) abi_fail(1, 7);
    if (!(a8 == (unsigned long long)abi_mix(108u, 0u))) abi_fail(1, 8);
    if (!(check_R7(&a9, 109u))) abi_fail(1, 9);
    if (!(check_R8(&a10, 110u))) abi_fail(1, 10);
    if (!(check_R9(&a11, 111u))) abi_fail(1, 11);
    if (!(check_R11(&a12, 112u))) abi_fail(1, 12);
}

float abi_f2(void) {
    float r = (float)((double)(long long)(abi_mix(299u, 0u) % 4001u) / 8.0 - 250.0);
    return r;
}

struct R15 abi_f3(float a0, float a1, struct R22 a2, _Bool a3, struct R23 a4, long long a5, double a6) {
    if (!(a0 == (float)((double)(long long)(abi_mix(300u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(3, 0);
    if (!(a1 == (float)((double)(long long)(abi_mix(301u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(3, 1);
    if (!(check_R22(&a2, 302u))) abi_fail(3, 2);
    if (!(a3 == (_Bool)(abi_mix(303u, 0u) & 1u))) abi_fail(3, 3);
    if (!(check_R23(&a4, 304u))) abi_fail(3, 4);
    if (!(a5 == (long long)abi_mix(305u, 0u))) abi_fail(3, 5);
    if (!(a6 == (double)((double)(long long)(abi_mix(306u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(3, 6);
    struct R15 r; fill_R15(&r, 399u);
    return r;
}

struct R24 abi_f4(struct R25 a0) {
    if (!(check_R25(&a0, 400u))) abi_fail(4, 0);
    struct R24 r; fill_R24(&r, 499u);
    return r;
}

static void abi_v5(va_list ap, int from) {
    if (from <= 0) {
        struct R31 v = va_arg(ap, struct R31);
        if (!(check_R31(&v, 550u))) abi_fail(5, 200);
    }
    if (from <= 1) {
        struct R33 v = va_arg(ap, struct R33);
        if (!(check_R33(&v, 551u))) abi_fail(5, 201);
    }
    if (from <= 2) {
        struct R34 v = va_arg(ap, struct R34);
        if (!(check_R34(&v, 552u))) abi_fail(5, 202);
    }
}
struct R26 abi_f5(union R28 a0, struct R30 a1, ...) {
    if (!(check_R28(&a0, 500u))) abi_fail(5, 0);
    if (!(check_R30(&a1, 501u))) abi_fail(5, 1);
    va_list ap, aq, ar;
    va_start(ap, a1);
    {
        struct R31 v = va_arg(ap, struct R31);
        if (!(check_R31(&v, 550u))) abi_fail(5, 100);
    }
    {
        struct R33 v = va_arg(ap, struct R33);
        if (!(check_R33(&v, 551u))) abi_fail(5, 101);
    }
    va_copy(aq, ap);
    {
        struct R34 v = va_arg(ap, struct R34);
        if (!(check_R34(&v, 552u))) abi_fail(5, 102);
    }
    va_end(ap);
    va_copy(ar, aq);
    abi_v5(aq, 2);
    va_end(aq);
    abi_v5(ar, 2);
    va_end(ar);
    struct R26 r; fill_R26(&r, 599u);
    return r;
}

static void abi_v6(va_list ap, int from) {
    if (from <= 0) {
        union R50 v = va_arg(ap, union R50);
        if (!(check_R50(&v, 650u))) abi_fail(6, 200);
    }
    if (from <= 1) {
        struct R51 v = va_arg(ap, struct R51);
        if (!(check_R51(&v, 651u))) abi_fail(6, 201);
    }
    if (from <= 2) {
        struct R52 v = va_arg(ap, struct R52);
        if (!(check_R52(&v, 652u))) abi_fail(6, 202);
    }
    if (from <= 3) {
        struct R53 v = va_arg(ap, struct R53);
        if (!(check_R53(&v, 653u))) abi_fail(6, 203);
    }
    if (from <= 4) {
        struct R54 v = va_arg(ap, struct R54);
        if (!(check_R54(&v, 654u))) abi_fail(6, 204);
    }
    if (from <= 5) {
        struct R55 v = va_arg(ap, struct R55);
        if (!(check_R55(&v, 655u))) abi_fail(6, 205);
    }
}
struct R39 abi_f6(char * a0, struct R41 a1, _Bool a2, char * a3, struct R42 a4, struct R44 a5, struct R46 a6, struct R47 a7, ...) {
    if (!(a0 == (abi_anchor + (abi_mix(600u, 0u) % 8u)))) abi_fail(6, 0);
    if (!(check_R41(&a1, 601u))) abi_fail(6, 1);
    if (!(a2 == (_Bool)(abi_mix(602u, 0u) & 1u))) abi_fail(6, 2);
    if (!(a3 == (abi_anchor + (abi_mix(603u, 0u) % 8u)))) abi_fail(6, 3);
    if (!(check_R42(&a4, 604u))) abi_fail(6, 4);
    if (!(check_R44(&a5, 605u))) abi_fail(6, 5);
    if (!(check_R46(&a6, 606u))) abi_fail(6, 6);
    if (!(check_R47(&a7, 607u))) abi_fail(6, 7);
    va_list ap, aq, ar;
    va_start(ap, a7);
    {
        union R50 v = va_arg(ap, union R50);
        if (!(check_R50(&v, 650u))) abi_fail(6, 100);
    }
    {
        struct R51 v = va_arg(ap, struct R51);
        if (!(check_R51(&v, 651u))) abi_fail(6, 101);
    }
    {
        struct R52 v = va_arg(ap, struct R52);
        if (!(check_R52(&v, 652u))) abi_fail(6, 102);
    }
    {
        struct R53 v = va_arg(ap, struct R53);
        if (!(check_R53(&v, 653u))) abi_fail(6, 103);
    }
    {
        struct R54 v = va_arg(ap, struct R54);
        if (!(check_R54(&v, 654u))) abi_fail(6, 104);
    }
    va_copy(aq, ap);
    {
        struct R55 v = va_arg(ap, struct R55);
        if (!(check_R55(&v, 655u))) abi_fail(6, 105);
    }
    va_end(ap);
    va_copy(ar, aq);
    abi_v6(aq, 5);
    va_end(aq);
    abi_v6(ar, 5);
    va_end(ar);
    struct R39 r; fill_R39(&r, 699u);
    return r;
}

static void abi_v7(va_list ap, int from) {
    if (from <= 0) {
        union R62 v = va_arg(ap, union R62);
        if (!(check_R62(&v, 750u))) abi_fail(7, 200);
    }
    if (from <= 1) {
        struct R63 v = va_arg(ap, struct R63);
        if (!(check_R63(&v, 751u))) abi_fail(7, 201);
    }
    if (from <= 2) {
        struct R64 v = va_arg(ap, struct R64);
        if (!(check_R64(&v, 752u))) abi_fail(7, 202);
    }
    if (from <= 3) {
        struct R65 v = va_arg(ap, struct R65);
        if (!(check_R65(&v, 753u))) abi_fail(7, 203);
    }
}
long long abi_f7(double a0, unsigned a1, unsigned long long a2, struct R56 a3, int a4, double a5, union R57 a6, struct R58 a7, struct R59 a8, ...) {
    if (!(a0 == (double)((double)(long long)(abi_mix(700u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(7, 0);
    if (!(a1 == (unsigned)(abi_mix(701u, 0u) % 4294967296u))) abi_fail(7, 1);
    if (!(a2 == (unsigned long long)abi_mix(702u, 0u))) abi_fail(7, 2);
    if (!(check_R56(&a3, 703u))) abi_fail(7, 3);
    if (!(a4 == (int)((long long)(abi_mix(704u, 0u) % 4294967296u) - 2147483648))) abi_fail(7, 4);
    if (!(a5 == (double)((double)(long long)(abi_mix(705u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(7, 5);
    if (!(check_R57(&a6, 706u))) abi_fail(7, 6);
    if (!(check_R58(&a7, 707u))) abi_fail(7, 7);
    if (!(check_R59(&a8, 708u))) abi_fail(7, 8);
    va_list ap, aq, ar;
    va_start(ap, a8);
    {
        union R62 v = va_arg(ap, union R62);
        if (!(check_R62(&v, 750u))) abi_fail(7, 100);
    }
    {
        struct R63 v = va_arg(ap, struct R63);
        if (!(check_R63(&v, 751u))) abi_fail(7, 101);
    }
    va_copy(aq, ap);
    {
        struct R64 v = va_arg(ap, struct R64);
        if (!(check_R64(&v, 752u))) abi_fail(7, 102);
    }
    {
        struct R65 v = va_arg(ap, struct R65);
        if (!(check_R65(&v, 753u))) abi_fail(7, 103);
    }
    va_end(ap);
    va_copy(ar, aq);
    abi_v7(aq, 2);
    va_end(aq);
    abi_v7(ar, 2);
    va_end(ar);
    long long r = (long long)abi_mix(799u, 0u);
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
    case 12: return sizeof(struct R12) * 1000 + _Alignof(struct R12);
    case 13: return sizeof(struct R13) * 1000 + _Alignof(struct R13);
    case 14: return sizeof(union R14) * 1000 + _Alignof(union R14);
    case 15: return sizeof(struct R15) * 1000 + _Alignof(struct R15);
    case 16: return sizeof(struct R16) * 1000 + _Alignof(struct R16);
    case 17: return sizeof(union R17) * 1000 + _Alignof(union R17);
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
    case 28: return sizeof(union R28) * 1000 + _Alignof(union R28);
    case 29: return sizeof(struct R29) * 1000 + _Alignof(struct R29);
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
    case 41: return sizeof(struct R41) * 1000 + _Alignof(struct R41);
    case 42: return sizeof(struct R42) * 1000 + _Alignof(struct R42);
    case 43: return sizeof(struct R43) * 1000 + _Alignof(struct R43);
    case 44: return sizeof(struct R44) * 1000 + _Alignof(struct R44);
    case 45: return sizeof(struct R45) * 1000 + _Alignof(struct R45);
    case 46: return sizeof(struct R46) * 1000 + _Alignof(struct R46);
    case 47: return sizeof(struct R47) * 1000 + _Alignof(struct R47);
    case 48: return sizeof(struct R48) * 1000 + _Alignof(struct R48);
    case 49: return sizeof(struct R49) * 1000 + _Alignof(struct R49);
    case 50: return sizeof(union R50) * 1000 + _Alignof(union R50);
    case 51: return sizeof(struct R51) * 1000 + _Alignof(struct R51);
    case 52: return sizeof(struct R52) * 1000 + _Alignof(struct R52);
    case 53: return sizeof(struct R53) * 1000 + _Alignof(struct R53);
    case 54: return sizeof(struct R54) * 1000 + _Alignof(struct R54);
    case 55: return sizeof(struct R55) * 1000 + _Alignof(struct R55);
    case 56: return sizeof(struct R56) * 1000 + _Alignof(struct R56);
    case 57: return sizeof(union R57) * 1000 + _Alignof(union R57);
    case 58: return sizeof(struct R58) * 1000 + _Alignof(struct R58);
    case 59: return sizeof(struct R59) * 1000 + _Alignof(struct R59);
    case 60: return sizeof(struct R60) * 1000 + _Alignof(struct R60);
    case 61: return sizeof(struct R61) * 1000 + _Alignof(struct R61);
    case 62: return sizeof(union R62) * 1000 + _Alignof(union R62);
    case 63: return sizeof(struct R63) * 1000 + _Alignof(struct R63);
    case 64: return sizeof(struct R64) * 1000 + _Alignof(struct R64);
    case 65: return sizeof(struct R65) * 1000 + _Alignof(struct R65);
    }
    return 0;
}
