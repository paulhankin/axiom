/* abi_gen_103_callee.c - written by gen_c_abi.py 103 8: the functions
   that abi_gen_103.c calls, which check their arguments. */
#include "abi_gen_103.h"

char abi_anchor[8];
int abi_failures;
int abi_first_test = -1, abi_first_what;

void abi_fail(int test, int what) {
    if (abi_failures++ == 0) {
        abi_first_test = test;
        abi_first_what = what;
    }
}

struct R1 abi_f0(unsigned short a0, struct R4 a1, union R5 a2) {
    if (!(a0 == (unsigned short)(abi_mix(0u, 0u) % 65536u))) abi_fail(0, 0);
    if (!(check_R4(&a1, 1u))) abi_fail(0, 1);
    if (!(check_R5(&a2, 2u))) abi_fail(0, 2);
    struct R1 r; fill_R1(&r, 99u);
    return r;
}

static void abi_v1(va_list ap, int from) {
    if (from <= 0) {
        struct R9 v = va_arg(ap, struct R9);
        if (!(check_R9(&v, 150u))) abi_fail(1, 200);
    }
    if (from <= 1) {
        struct R10 v = va_arg(ap, struct R10);
        if (!(check_R10(&v, 151u))) abi_fail(1, 201);
    }
    if (from <= 2) {
        union R14 v = va_arg(ap, union R14);
        if (!(check_R14(&v, 152u))) abi_fail(1, 202);
    }
}
struct R7 abi_f1(struct R8 a0, float a1, ...) {
    if (!(check_R8(&a0, 100u))) abi_fail(1, 0);
    if (!(a1 == (float)((double)(long long)(abi_mix(101u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(1, 1);
    va_list ap, aq, ar;
    va_start(ap, a1);
    {
        struct R9 v = va_arg(ap, struct R9);
        if (!(check_R9(&v, 150u))) abi_fail(1, 100);
    }
    {
        struct R10 v = va_arg(ap, struct R10);
        if (!(check_R10(&v, 151u))) abi_fail(1, 101);
    }
    va_copy(aq, ap);
    {
        union R14 v = va_arg(ap, union R14);
        if (!(check_R14(&v, 152u))) abi_fail(1, 102);
    }
    va_end(ap);
    va_copy(ar, aq);
    abi_v1(aq, 2);
    va_end(aq);
    abi_v1(ar, 2);
    va_end(ar);
    struct R7 r; fill_R7(&r, 199u);
    return r;
}

union R17 abi_f2(struct R18 a0) {
    if (!(check_R18(&a0, 200u))) abi_fail(2, 0);
    union R17 r; fill_R17(&r, 299u);
    return r;
}

static void abi_v3(va_list ap, int from) {
    if (from <= 0) {
        struct R24 v = va_arg(ap, struct R24);
        if (!(check_R24(&v, 350u))) abi_fail(3, 200);
    }
    if (from <= 1) {
        struct R26 v = va_arg(ap, struct R26);
        if (!(check_R26(&v, 351u))) abi_fail(3, 201);
    }
}
struct R19 abi_f3(int a0, struct R21 a1, unsigned long long a2, struct R22 a3, ...) {
    if (!(a0 == (int)((long long)(abi_mix(300u, 0u) % 4294967296u) - 2147483648))) abi_fail(3, 0);
    if (!(check_R21(&a1, 301u))) abi_fail(3, 1);
    if (!(a2 == (unsigned long long)abi_mix(302u, 0u))) abi_fail(3, 2);
    if (!(check_R22(&a3, 303u))) abi_fail(3, 3);
    va_list ap, aq, ar;
    va_start(ap, a3);
    va_copy(aq, ap);
    {
        struct R24 v = va_arg(ap, struct R24);
        if (!(check_R24(&v, 350u))) abi_fail(3, 100);
    }
    {
        struct R26 v = va_arg(ap, struct R26);
        if (!(check_R26(&v, 351u))) abi_fail(3, 101);
    }
    va_end(ap);
    va_copy(ar, aq);
    abi_v3(aq, 0);
    va_end(aq);
    abi_v3(ar, 0);
    va_end(ar);
    struct R19 r; fill_R19(&r, 399u);
    return r;
}

struct R27 abi_f4(short a0, float a1, struct R28 a2, int a3, struct R30 a4) {
    if (!(a0 == (short)((long long)(abi_mix(400u, 0u) % 65536u) - 32768))) abi_fail(4, 0);
    if (!(a1 == (float)((double)(long long)(abi_mix(401u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(4, 1);
    if (!(check_R28(&a2, 402u))) abi_fail(4, 2);
    if (!(a3 == (int)((long long)(abi_mix(403u, 0u) % 4294967296u) - 2147483648))) abi_fail(4, 3);
    if (!(check_R30(&a4, 404u))) abi_fail(4, 4);
    struct R27 r; fill_R27(&r, 499u);
    return r;
}

void abi_f5(float a0, int a1, double a2, int a3, double a4, double a5, long long a6, long long a7, struct R32 a8) {
    if (!(a0 == (float)((double)(long long)(abi_mix(500u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(5, 0);
    if (!(a1 == (int)((long long)(abi_mix(501u, 0u) % 4294967296u) - 2147483648))) abi_fail(5, 1);
    if (!(a2 == (double)((double)(long long)(abi_mix(502u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(5, 2);
    if (!(a3 == (int)((long long)(abi_mix(503u, 0u) % 4294967296u) - 2147483648))) abi_fail(5, 3);
    if (!(a4 == (double)((double)(long long)(abi_mix(504u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(5, 4);
    if (!(a5 == (double)((double)(long long)(abi_mix(505u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(5, 5);
    if (!(a6 == (long long)abi_mix(506u, 0u))) abi_fail(5, 6);
    if (!(a7 == (long long)abi_mix(507u, 0u))) abi_fail(5, 7);
    if (!(check_R32(&a8, 508u))) abi_fail(5, 8);
}

static void abi_v6(va_list ap, int from) {
    if (from <= 0) {
        int v = va_arg(ap, int);
        if (!(v == (int)((long long)(abi_mix(650u, 0u) % 4294967296u) - 2147483648))) abi_fail(6, 200);
    }
}
float abi_f6(union R38 a0, union R39 a1, struct R40 a2, ...) {
    if (!(check_R38(&a0, 600u))) abi_fail(6, 0);
    if (!(check_R39(&a1, 601u))) abi_fail(6, 1);
    if (!(check_R40(&a2, 602u))) abi_fail(6, 2);
    va_list ap, aq, ar;
    va_start(ap, a2);
    va_copy(aq, ap);
    {
        int v = va_arg(ap, int);
        if (!(v == (int)((long long)(abi_mix(650u, 0u) % 4294967296u) - 2147483648))) abi_fail(6, 100);
    }
    va_end(ap);
    va_copy(ar, aq);
    abi_v6(aq, 0);
    va_end(aq);
    abi_v6(ar, 0);
    va_end(ar);
    float r = (float)((double)(long long)(abi_mix(699u, 0u) % 4001u) / 8.0 - 250.0);
    return r;
}

struct R43 abi_f7(struct R45 a0, short a1, struct R46 a2, struct R47 a3) {
    if (!(check_R45(&a0, 700u))) abi_fail(7, 0);
    if (!(a1 == (short)((long long)(abi_mix(701u, 0u) % 65536u) - 32768))) abi_fail(7, 1);
    if (!(check_R46(&a2, 702u))) abi_fail(7, 2);
    if (!(check_R47(&a3, 703u))) abi_fail(7, 3);
    struct R43 r; fill_R43(&r, 799u);
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
    case 5: return sizeof(union R5) * 1000 + _Alignof(union R5);
    case 6: return sizeof(struct R6) * 1000 + _Alignof(struct R6);
    case 7: return sizeof(struct R7) * 1000 + _Alignof(struct R7);
    case 8: return sizeof(struct R8) * 1000 + _Alignof(struct R8);
    case 9: return sizeof(struct R9) * 1000 + _Alignof(struct R9);
    case 10: return sizeof(struct R10) * 1000 + _Alignof(struct R10);
    case 11: return sizeof(struct R11) * 1000 + _Alignof(struct R11);
    case 12: return sizeof(struct R12) * 1000 + _Alignof(struct R12);
    case 13: return sizeof(union R13) * 1000 + _Alignof(union R13);
    case 14: return sizeof(union R14) * 1000 + _Alignof(union R14);
    case 15: return sizeof(struct R15) * 1000 + _Alignof(struct R15);
    case 16: return sizeof(struct R16) * 1000 + _Alignof(struct R16);
    case 17: return sizeof(union R17) * 1000 + _Alignof(union R17);
    case 18: return sizeof(struct R18) * 1000 + _Alignof(struct R18);
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
    case 39: return sizeof(union R39) * 1000 + _Alignof(union R39);
    case 40: return sizeof(struct R40) * 1000 + _Alignof(struct R40);
    case 41: return sizeof(struct R41) * 1000 + _Alignof(struct R41);
    case 42: return sizeof(struct R42) * 1000 + _Alignof(struct R42);
    case 43: return sizeof(struct R43) * 1000 + _Alignof(struct R43);
    case 44: return sizeof(struct R44) * 1000 + _Alignof(struct R44);
    case 45: return sizeof(struct R45) * 1000 + _Alignof(struct R45);
    case 46: return sizeof(struct R46) * 1000 + _Alignof(struct R46);
    case 47: return sizeof(struct R47) * 1000 + _Alignof(struct R47);
    }
    return 0;
}
