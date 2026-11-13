/* abi_avx_callee.c - written by gen_c_abi.py 31 8: the functions
   that abi_avx.c calls, which check their arguments. */
#include "abi_avx.h"

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
        struct R1 v = va_arg(ap, struct R1);
        if (!(check_R1(&v, 50u))) abi_fail(0, 200);
    }
    if (from <= 1) {
        struct R3 v = va_arg(ap, struct R3);
        if (!(check_R3(&v, 51u))) abi_fail(0, 201);
    }
}
void abi_f0(double a0, abi_v2d a1, int a2, double a3, int a4, abi_v4i a5, double a6, unsigned char a7, struct R0 a8, ...) {
    if (!(a0 == (double)((double)(long long)(abi_mix(0u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(0, 0);
    if (!(check_abi_v2d(&a1, 1u))) abi_fail(0, 1);
    if (!(a2 == (int)((long long)(abi_mix(2u, 0u) % 4294967296u) - 2147483648))) abi_fail(0, 2);
    if (!(a3 == (double)((double)(long long)(abi_mix(3u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(0, 3);
    if (!(a4 == (int)((long long)(abi_mix(4u, 0u) % 4294967296u) - 2147483648))) abi_fail(0, 4);
    if (!(check_abi_v4i(&a5, 5u))) abi_fail(0, 5);
    if (!(a6 == (double)((double)(long long)(abi_mix(6u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(0, 6);
    if (!(a7 == (unsigned char)(abi_mix(7u, 0u) % 256u))) abi_fail(0, 7);
    if (!(check_R0(&a8, 8u))) abi_fail(0, 8);
    va_list ap, aq, ar;
    va_start(ap, a8);
    {
        struct R1 v = va_arg(ap, struct R1);
        if (!(check_R1(&v, 50u))) abi_fail(0, 100);
    }
    va_copy(aq, ap);
    {
        struct R3 v = va_arg(ap, struct R3);
        if (!(check_R3(&v, 51u))) abi_fail(0, 101);
    }
    va_end(ap);
    va_copy(ar, aq);
    abi_v0(aq, 1);
    va_end(aq);
    abi_v0(ar, 1);
    va_end(ar);
}

void abi_f1(struct R4 a0, unsigned short a1, struct R5 a2) {
    if (!(check_R4(&a0, 100u))) abi_fail(1, 0);
    if (!(a1 == (unsigned short)(abi_mix(101u, 0u) % 65536u))) abi_fail(1, 1);
    if (!(check_R5(&a2, 102u))) abi_fail(1, 2);
}

static void abi_v2(va_list ap, int from) {
    if (from <= 0) {
        struct R7 v = va_arg(ap, struct R7);
        if (!(check_R7(&v, 250u))) abi_fail(2, 200);
    }
    if (from <= 1) {
        unsigned v = va_arg(ap, unsigned);
        if (!(v == (unsigned)(abi_mix(251u, 0u) % 4294967296u))) abi_fail(2, 201);
    }
    if (from <= 2) {
        struct R9 v = va_arg(ap, struct R9);
        if (!(check_R9(&v, 252u))) abi_fail(2, 202);
    }
    if (from <= 3) {
        union R10 v = va_arg(ap, union R10);
        if (!(check_R10(&v, 253u))) abi_fail(2, 203);
    }
    if (from <= 4) {
        struct R11 v = va_arg(ap, struct R11);
        if (!(check_R11(&v, 254u))) abi_fail(2, 204);
    }
}
struct R6 abi_f2(int a0, ...) {
    if (!(a0 == (int)((long long)(abi_mix(200u, 0u) % 4294967296u) - 2147483648))) abi_fail(2, 0);
    va_list ap, aq, ar;
    va_start(ap, a0);
    {
        struct R7 v = va_arg(ap, struct R7);
        if (!(check_R7(&v, 250u))) abi_fail(2, 100);
    }
    {
        unsigned v = va_arg(ap, unsigned);
        if (!(v == (unsigned)(abi_mix(251u, 0u) % 4294967296u))) abi_fail(2, 101);
    }
    va_copy(aq, ap);
    {
        struct R9 v = va_arg(ap, struct R9);
        if (!(check_R9(&v, 252u))) abi_fail(2, 102);
    }
    {
        union R10 v = va_arg(ap, union R10);
        if (!(check_R10(&v, 253u))) abi_fail(2, 103);
    }
    {
        struct R11 v = va_arg(ap, struct R11);
        if (!(check_R11(&v, 254u))) abi_fail(2, 104);
    }
    va_end(ap);
    va_copy(ar, aq);
    abi_v2(aq, 2);
    va_end(aq);
    abi_v2(ar, 2);
    va_end(ar);
    struct R6 r; fill_R6(&r, 299u);
    return r;
}

struct R12 abi_f3(abi_v2d a0, unsigned short a1, struct R13 a2, abi_v4i a3, struct R15 a4, struct R17 a5) {
    if (!(check_abi_v2d(&a0, 300u))) abi_fail(3, 0);
    if (!(a1 == (unsigned short)(abi_mix(301u, 0u) % 65536u))) abi_fail(3, 1);
    if (!(check_R13(&a2, 302u))) abi_fail(3, 2);
    if (!(check_abi_v4i(&a3, 303u))) abi_fail(3, 3);
    if (!(check_R15(&a4, 304u))) abi_fail(3, 4);
    if (!(check_R17(&a5, 305u))) abi_fail(3, 5);
    struct R12 r; fill_R12(&r, 399u);
    return r;
}

struct R18 abi_f4(double a0, int a1, unsigned a2, struct R20 a3, signed char a4, struct R21 a5, struct R22 a6, struct R23 a7, abi_v8i a8, struct R24 a9, abi_v4c a10, abi_v8uc a11) {
    if (!(a0 == (double)((double)(long long)(abi_mix(400u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(4, 0);
    if (!(a1 == (int)((long long)(abi_mix(401u, 0u) % 4294967296u) - 2147483648))) abi_fail(4, 1);
    if (!(a2 == (unsigned)(abi_mix(402u, 0u) % 4294967296u))) abi_fail(4, 2);
    if (!(check_R20(&a3, 403u))) abi_fail(4, 3);
    if (!(a4 == (signed char)((long long)(abi_mix(404u, 0u) % 256u) - 128))) abi_fail(4, 4);
    if (!(check_R21(&a5, 405u))) abi_fail(4, 5);
    if (!(check_R22(&a6, 406u))) abi_fail(4, 6);
    if (!(check_R23(&a7, 407u))) abi_fail(4, 7);
    if (!(check_abi_v8i(&a8, 408u))) abi_fail(4, 8);
    if (!(check_R24(&a9, 409u))) abi_fail(4, 9);
    if (!(check_abi_v4c(&a10, 410u))) abi_fail(4, 10);
    if (!(check_abi_v8uc(&a11, 411u))) abi_fail(4, 11);
    struct R18 r; fill_R18(&r, 499u);
    return r;
}

struct R25 abi_f5(abi_v4i a0, short a1, struct R26 a2, struct R27 a3, struct R28 a4, char * a5, union R29 a6, struct R30 a7) {
    if (!(check_abi_v4i(&a0, 500u))) abi_fail(5, 0);
    if (!(a1 == (short)((long long)(abi_mix(501u, 0u) % 65536u) - 32768))) abi_fail(5, 1);
    if (!(check_R26(&a2, 502u))) abi_fail(5, 2);
    if (!(check_R27(&a3, 503u))) abi_fail(5, 3);
    if (!(check_R28(&a4, 504u))) abi_fail(5, 4);
    if (!(a5 == (abi_anchor + (abi_mix(505u, 0u) % 8u)))) abi_fail(5, 5);
    if (!(check_R29(&a6, 506u))) abi_fail(5, 6);
    if (!(check_R30(&a7, 507u))) abi_fail(5, 7);
    struct R25 r; fill_R25(&r, 599u);
    return r;
}

union R31 abi_f6(_Bool a0, struct R32 a1, _Bool a2, abi_v4f a3, unsigned char a4, struct R33 a5, struct R34 a6, abi_v4c a7, struct R35 a8, struct R36 a9) {
    if (!(a0 == (_Bool)(abi_mix(600u, 0u) & 1u))) abi_fail(6, 0);
    if (!(check_R32(&a1, 601u))) abi_fail(6, 1);
    if (!(a2 == (_Bool)(abi_mix(602u, 0u) & 1u))) abi_fail(6, 2);
    if (!(check_abi_v4f(&a3, 603u))) abi_fail(6, 3);
    if (!(a4 == (unsigned char)(abi_mix(604u, 0u) % 256u))) abi_fail(6, 4);
    if (!(check_R33(&a5, 605u))) abi_fail(6, 5);
    if (!(check_R34(&a6, 606u))) abi_fail(6, 6);
    if (!(check_abi_v4c(&a7, 607u))) abi_fail(6, 7);
    if (!(check_R35(&a8, 608u))) abi_fail(6, 8);
    if (!(check_R36(&a9, 609u))) abi_fail(6, 9);
    union R31 r; fill_R31(&r, 699u);
    return r;
}

static void abi_v7(va_list ap, int from) {
    if (from <= 0) {
        abi_v4i v = va_arg(ap, abi_v4i);
        if (!(check_abi_v4i(&v, 750u))) abi_fail(7, 200);
    }
    if (from <= 1) {
        double v = va_arg(ap, double);
        if (!(v == (double)((double)(long long)(abi_mix(751u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(7, 201);
    }
    if (from <= 2) {
        abi_v8s v = va_arg(ap, abi_v8s);
        if (!(check_abi_v8s(&v, 752u))) abi_fail(7, 202);
    }
    if (from <= 3) {
        struct R38 v = va_arg(ap, struct R38);
        if (!(check_R38(&v, 753u))) abi_fail(7, 203);
    }
    if (from <= 4) {
        struct R39 v = va_arg(ap, struct R39);
        if (!(check_R39(&v, 754u))) abi_fail(7, 204);
    }
    if (from <= 5) {
        abi_v4d v = va_arg(ap, abi_v4d);
        if (!(check_abi_v4d(&v, 755u))) abi_fail(7, 205);
    }
    if (from <= 6) {
        union R40 v = va_arg(ap, union R40);
        if (!(check_R40(&v, 756u))) abi_fail(7, 206);
    }
}
struct R37 abi_f7(float a0, float a1, unsigned long long a2, ...) {
    if (!(a0 == (float)((double)(long long)(abi_mix(700u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(7, 0);
    if (!(a1 == (float)((double)(long long)(abi_mix(701u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(7, 1);
    if (!(a2 == (unsigned long long)abi_mix(702u, 0u))) abi_fail(7, 2);
    va_list ap, aq, ar;
    va_start(ap, a2);
    va_copy(aq, ap);
    {
        abi_v4i v = va_arg(ap, abi_v4i);
        if (!(check_abi_v4i(&v, 750u))) abi_fail(7, 100);
    }
    {
        double v = va_arg(ap, double);
        if (!(v == (double)((double)(long long)(abi_mix(751u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(7, 101);
    }
    {
        abi_v8s v = va_arg(ap, abi_v8s);
        if (!(check_abi_v8s(&v, 752u))) abi_fail(7, 102);
    }
    {
        struct R38 v = va_arg(ap, struct R38);
        if (!(check_R38(&v, 753u))) abi_fail(7, 103);
    }
    {
        struct R39 v = va_arg(ap, struct R39);
        if (!(check_R39(&v, 754u))) abi_fail(7, 104);
    }
    {
        abi_v4d v = va_arg(ap, abi_v4d);
        if (!(check_abi_v4d(&v, 755u))) abi_fail(7, 105);
    }
    {
        union R40 v = va_arg(ap, union R40);
        if (!(check_R40(&v, 756u))) abi_fail(7, 106);
    }
    va_end(ap);
    va_copy(ar, aq);
    abi_v7(aq, 0);
    va_end(aq);
    abi_v7(ar, 0);
    va_end(ar);
    struct R37 r; fill_R37(&r, 799u);
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
    case 10: return sizeof(union R10) * 1000 + _Alignof(union R10);
    case 11: return sizeof(struct R11) * 1000 + _Alignof(struct R11);
    case 12: return sizeof(struct R12) * 1000 + _Alignof(struct R12);
    case 13: return sizeof(struct R13) * 1000 + _Alignof(struct R13);
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
    case 26: return sizeof(struct R26) * 1000 + _Alignof(struct R26);
    case 27: return sizeof(struct R27) * 1000 + _Alignof(struct R27);
    case 28: return sizeof(struct R28) * 1000 + _Alignof(struct R28);
    case 29: return sizeof(union R29) * 1000 + _Alignof(union R29);
    case 30: return sizeof(struct R30) * 1000 + _Alignof(struct R30);
    case 31: return sizeof(union R31) * 1000 + _Alignof(union R31);
    case 32: return sizeof(struct R32) * 1000 + _Alignof(struct R32);
    case 33: return sizeof(struct R33) * 1000 + _Alignof(struct R33);
    case 34: return sizeof(struct R34) * 1000 + _Alignof(struct R34);
    case 35: return sizeof(struct R35) * 1000 + _Alignof(struct R35);
    case 36: return sizeof(struct R36) * 1000 + _Alignof(struct R36);
    case 37: return sizeof(struct R37) * 1000 + _Alignof(struct R37);
    case 38: return sizeof(struct R38) * 1000 + _Alignof(struct R38);
    case 39: return sizeof(struct R39) * 1000 + _Alignof(struct R39);
    case 40: return sizeof(union R40) * 1000 + _Alignof(union R40);
    }
    return 0;
}
