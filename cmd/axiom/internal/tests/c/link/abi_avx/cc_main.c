/* The caller of ourlib_callee.c, compiled by the system's compiler (main.c). */
#include <stdio.h>
void print_i64(long long x) { printf("%lld\n", x); }
void print_i64(long long x);
#include "abi_avx.h"

int abi_result(void);
int abi_first(void);
unsigned long long abi_layout(int k);

static void (*volatile abi_p0)(double a0, abi_v2d a1, int a2, double a3, int a4, abi_v4i a5, double a6, unsigned char a7, struct R0 a8, ...) = abi_f0;
static void (*volatile abi_p1)(struct R4 a0, unsigned short a1, struct R5 a2) = abi_f1;
static struct R6 (*volatile abi_p2)(int a0, ...) = abi_f2;
static struct R12 (*volatile abi_p3)(abi_v2d a0, unsigned short a1, struct R13 a2, abi_v4i a3, struct R15 a4, struct R17 a5) = abi_f3;
static struct R18 (*volatile abi_p4)(double a0, int a1, unsigned a2, struct R20 a3, signed char a4, struct R21 a5, struct R22 a6, struct R23 a7, abi_v8i a8, struct R24 a9, abi_v4c a10, abi_v8uc a11) = abi_f4;
static struct R25 (*volatile abi_p5)(abi_v4i a0, short a1, struct R26 a2, struct R27 a3, struct R28 a4, char * a5, union R29 a6, struct R30 a7) = abi_f5;
static union R31 (*volatile abi_p6)(_Bool a0, struct R32 a1, _Bool a2, abi_v4f a3, unsigned char a4, struct R33 a5, struct R34 a6, abi_v4c a7, struct R35 a8, struct R36 a9) = abi_f6;
static struct R37 (*volatile abi_p7)(float a0, float a1, unsigned long long a2, ...) = abi_f7;

static int abi_t0(void) {
    double a0 = (double)((double)(long long)(abi_mix(0u, 0u) % 4001u) / 8.0 - 250.0);
    abi_v2d a1; fill_abi_v2d(&a1, 1u);
    int a2 = (int)((long long)(abi_mix(2u, 0u) % 4294967296u) - 2147483648);
    double a3 = (double)((double)(long long)(abi_mix(3u, 0u) % 4001u) / 8.0 - 250.0);
    int a4 = (int)((long long)(abi_mix(4u, 0u) % 4294967296u) - 2147483648);
    abi_v4i a5; fill_abi_v4i(&a5, 5u);
    double a6 = (double)((double)(long long)(abi_mix(6u, 0u) % 4001u) / 8.0 - 250.0);
    unsigned char a7 = (unsigned char)(abi_mix(7u, 0u) % 256u);
    struct R0 a8; fill_R0(&a8, 8u);
    struct R1 v0; fill_R1(&v0, 50u);
    struct R3 v1; fill_R3(&v1, 51u);
    int bad = 0;
    abi_f0(a0, a1, a2, a3, a4, a5, a6, a7, a8, v0, v1);
    abi_p0(a0, a1, a2, a3, a4, a5, a6, a7, a8, v0, v1);
    if (bad) abi_fail(0, 99);
    return 2;
}

static int abi_t1(void) {
    struct R4 a0; fill_R4(&a0, 100u);
    unsigned short a1 = (unsigned short)(abi_mix(101u, 0u) % 65536u);
    struct R5 a2; fill_R5(&a2, 102u);
    int bad = 0;
    abi_f1(a0, a1, a2);
    abi_p1(a0, a1, a2);
    if (bad) abi_fail(1, 99);
    return 2;
}

static int abi_t2(void) {
    int a0 = (int)((long long)(abi_mix(200u, 0u) % 4294967296u) - 2147483648);
    struct R7 v0; fill_R7(&v0, 250u);
    unsigned v1 = (unsigned)(abi_mix(251u, 0u) % 4294967296u);
    struct R9 v2; fill_R9(&v2, 252u);
    union R10 v3; fill_R10(&v3, 253u);
    struct R11 v4; fill_R11(&v4, 254u);
    int bad = 0;
    {
        struct R6 r = abi_f2(a0, v0, v1, v2, v3, v4);
        if (!(check_R6(&r, 299u))) bad = 1;
    }
    {
        struct R6 r = abi_p2(a0, v0, v1, v2, v3, v4);
        if (!(check_R6(&r, 299u))) bad = 1;
    }
    if (bad) abi_fail(2, 99);
    return 2;
}

static int abi_t3(void) {
    abi_v2d a0; fill_abi_v2d(&a0, 300u);
    unsigned short a1 = (unsigned short)(abi_mix(301u, 0u) % 65536u);
    struct R13 a2; fill_R13(&a2, 302u);
    abi_v4i a3; fill_abi_v4i(&a3, 303u);
    struct R15 a4; fill_R15(&a4, 304u);
    struct R17 a5; fill_R17(&a5, 305u);
    int bad = 0;
    {
        struct R12 r = abi_f3(a0, a1, a2, a3, a4, a5);
        if (!(check_R12(&r, 399u))) bad = 1;
    }
    {
        struct R12 r = abi_p3(a0, a1, a2, a3, a4, a5);
        if (!(check_R12(&r, 399u))) bad = 1;
    }
    if (bad) abi_fail(3, 99);
    return 2;
}

static int abi_t4(void) {
    double a0 = (double)((double)(long long)(abi_mix(400u, 0u) % 4001u) / 8.0 - 250.0);
    int a1 = (int)((long long)(abi_mix(401u, 0u) % 4294967296u) - 2147483648);
    unsigned a2 = (unsigned)(abi_mix(402u, 0u) % 4294967296u);
    struct R20 a3; fill_R20(&a3, 403u);
    signed char a4 = (signed char)((long long)(abi_mix(404u, 0u) % 256u) - 128);
    struct R21 a5; fill_R21(&a5, 405u);
    struct R22 a6; fill_R22(&a6, 406u);
    struct R23 a7; fill_R23(&a7, 407u);
    abi_v8i a8; fill_abi_v8i(&a8, 408u);
    struct R24 a9; fill_R24(&a9, 409u);
    abi_v4c a10; fill_abi_v4c(&a10, 410u);
    abi_v8uc a11; fill_abi_v8uc(&a11, 411u);
    int bad = 0;
    {
        struct R18 r = abi_f4(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
        if (!(check_R18(&r, 499u))) bad = 1;
    }
    {
        struct R18 r = abi_p4(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
        if (!(check_R18(&r, 499u))) bad = 1;
    }
    if (bad) abi_fail(4, 99);
    return 2;
}

static int abi_t5(void) {
    abi_v4i a0; fill_abi_v4i(&a0, 500u);
    short a1 = (short)((long long)(abi_mix(501u, 0u) % 65536u) - 32768);
    struct R26 a2; fill_R26(&a2, 502u);
    struct R27 a3; fill_R27(&a3, 503u);
    struct R28 a4; fill_R28(&a4, 504u);
    char * a5 = (abi_anchor + (abi_mix(505u, 0u) % 8u));
    union R29 a6; fill_R29(&a6, 506u);
    struct R30 a7; fill_R30(&a7, 507u);
    int bad = 0;
    {
        struct R25 r = abi_f5(a0, a1, a2, a3, a4, a5, a6, a7);
        if (!(check_R25(&r, 599u))) bad = 1;
    }
    {
        struct R25 r = abi_p5(a0, a1, a2, a3, a4, a5, a6, a7);
        if (!(check_R25(&r, 599u))) bad = 1;
    }
    if (bad) abi_fail(5, 99);
    return 2;
}

static int abi_t6(void) {
    _Bool a0 = (_Bool)(abi_mix(600u, 0u) & 1u);
    struct R32 a1; fill_R32(&a1, 601u);
    _Bool a2 = (_Bool)(abi_mix(602u, 0u) & 1u);
    abi_v4f a3; fill_abi_v4f(&a3, 603u);
    unsigned char a4 = (unsigned char)(abi_mix(604u, 0u) % 256u);
    struct R33 a5; fill_R33(&a5, 605u);
    struct R34 a6; fill_R34(&a6, 606u);
    abi_v4c a7; fill_abi_v4c(&a7, 607u);
    struct R35 a8; fill_R35(&a8, 608u);
    struct R36 a9; fill_R36(&a9, 609u);
    int bad = 0;
    {
        union R31 r = abi_f6(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        if (!(check_R31(&r, 699u))) bad = 1;
    }
    {
        union R31 r = abi_p6(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        if (!(check_R31(&r, 699u))) bad = 1;
    }
    if (bad) abi_fail(6, 99);
    return 2;
}

static int abi_t7(void) {
    float a0 = (float)((double)(long long)(abi_mix(700u, 0u) % 4001u) / 8.0 - 250.0);
    float a1 = (float)((double)(long long)(abi_mix(701u, 0u) % 4001u) / 8.0 - 250.0);
    unsigned long long a2 = (unsigned long long)abi_mix(702u, 0u);
    abi_v4i v0; fill_abi_v4i(&v0, 750u);
    double v1 = (double)((double)(long long)(abi_mix(751u, 0u) % 4001u) / 8.0 - 250.0);
    abi_v8s v2; fill_abi_v8s(&v2, 752u);
    struct R38 v3; fill_R38(&v3, 753u);
    struct R39 v4; fill_R39(&v4, 754u);
    abi_v4d v5; fill_abi_v4d(&v5, 755u);
    union R40 v6; fill_R40(&v6, 756u);
    int bad = 0;
    {
        struct R37 r = abi_f7(a0, a1, a2, v0, v1, v2, v3, v4, v5, v6);
        if (!(check_R37(&r, 799u))) bad = 1;
    }
    {
        struct R37 r = abi_p7(a0, a1, a2, v0, v1, v2, v3, v4, v5, v6);
        if (!(check_R37(&r, 799u))) bad = 1;
    }
    if (bad) abi_fail(7, 99);
    return 2;
}

int main(void) {
    int calls = 0;
    if (abi_layout(0) != sizeof(struct R0) * 1000 + _Alignof(struct R0)) abi_fail(-1, 0);
    if (abi_layout(1) != sizeof(struct R1) * 1000 + _Alignof(struct R1)) abi_fail(-1, 1);
    if (abi_layout(2) != sizeof(struct R2) * 1000 + _Alignof(struct R2)) abi_fail(-1, 2);
    if (abi_layout(3) != sizeof(struct R3) * 1000 + _Alignof(struct R3)) abi_fail(-1, 3);
    if (abi_layout(4) != sizeof(struct R4) * 1000 + _Alignof(struct R4)) abi_fail(-1, 4);
    if (abi_layout(5) != sizeof(struct R5) * 1000 + _Alignof(struct R5)) abi_fail(-1, 5);
    if (abi_layout(6) != sizeof(struct R6) * 1000 + _Alignof(struct R6)) abi_fail(-1, 6);
    if (abi_layout(7) != sizeof(struct R7) * 1000 + _Alignof(struct R7)) abi_fail(-1, 7);
    if (abi_layout(8) != sizeof(struct R8) * 1000 + _Alignof(struct R8)) abi_fail(-1, 8);
    if (abi_layout(9) != sizeof(struct R9) * 1000 + _Alignof(struct R9)) abi_fail(-1, 9);
    if (abi_layout(10) != sizeof(union R10) * 1000 + _Alignof(union R10)) abi_fail(-1, 10);
    if (abi_layout(11) != sizeof(struct R11) * 1000 + _Alignof(struct R11)) abi_fail(-1, 11);
    if (abi_layout(12) != sizeof(struct R12) * 1000 + _Alignof(struct R12)) abi_fail(-1, 12);
    if (abi_layout(13) != sizeof(struct R13) * 1000 + _Alignof(struct R13)) abi_fail(-1, 13);
    if (abi_layout(14) != sizeof(struct R14) * 1000 + _Alignof(struct R14)) abi_fail(-1, 14);
    if (abi_layout(15) != sizeof(struct R15) * 1000 + _Alignof(struct R15)) abi_fail(-1, 15);
    if (abi_layout(16) != sizeof(struct R16) * 1000 + _Alignof(struct R16)) abi_fail(-1, 16);
    if (abi_layout(17) != sizeof(struct R17) * 1000 + _Alignof(struct R17)) abi_fail(-1, 17);
    if (abi_layout(18) != sizeof(struct R18) * 1000 + _Alignof(struct R18)) abi_fail(-1, 18);
    if (abi_layout(19) != sizeof(struct R19) * 1000 + _Alignof(struct R19)) abi_fail(-1, 19);
    if (abi_layout(20) != sizeof(struct R20) * 1000 + _Alignof(struct R20)) abi_fail(-1, 20);
    if (abi_layout(21) != sizeof(struct R21) * 1000 + _Alignof(struct R21)) abi_fail(-1, 21);
    if (abi_layout(22) != sizeof(struct R22) * 1000 + _Alignof(struct R22)) abi_fail(-1, 22);
    if (abi_layout(23) != sizeof(struct R23) * 1000 + _Alignof(struct R23)) abi_fail(-1, 23);
    if (abi_layout(24) != sizeof(struct R24) * 1000 + _Alignof(struct R24)) abi_fail(-1, 24);
    if (abi_layout(25) != sizeof(struct R25) * 1000 + _Alignof(struct R25)) abi_fail(-1, 25);
    if (abi_layout(26) != sizeof(struct R26) * 1000 + _Alignof(struct R26)) abi_fail(-1, 26);
    if (abi_layout(27) != sizeof(struct R27) * 1000 + _Alignof(struct R27)) abi_fail(-1, 27);
    if (abi_layout(28) != sizeof(struct R28) * 1000 + _Alignof(struct R28)) abi_fail(-1, 28);
    if (abi_layout(29) != sizeof(union R29) * 1000 + _Alignof(union R29)) abi_fail(-1, 29);
    if (abi_layout(30) != sizeof(struct R30) * 1000 + _Alignof(struct R30)) abi_fail(-1, 30);
    if (abi_layout(31) != sizeof(union R31) * 1000 + _Alignof(union R31)) abi_fail(-1, 31);
    if (abi_layout(32) != sizeof(struct R32) * 1000 + _Alignof(struct R32)) abi_fail(-1, 32);
    if (abi_layout(33) != sizeof(struct R33) * 1000 + _Alignof(struct R33)) abi_fail(-1, 33);
    if (abi_layout(34) != sizeof(struct R34) * 1000 + _Alignof(struct R34)) abi_fail(-1, 34);
    if (abi_layout(35) != sizeof(struct R35) * 1000 + _Alignof(struct R35)) abi_fail(-1, 35);
    if (abi_layout(36) != sizeof(struct R36) * 1000 + _Alignof(struct R36)) abi_fail(-1, 36);
    if (abi_layout(37) != sizeof(struct R37) * 1000 + _Alignof(struct R37)) abi_fail(-1, 37);
    if (abi_layout(38) != sizeof(struct R38) * 1000 + _Alignof(struct R38)) abi_fail(-1, 38);
    if (abi_layout(39) != sizeof(struct R39) * 1000 + _Alignof(struct R39)) abi_fail(-1, 39);
    if (abi_layout(40) != sizeof(union R40) * 1000 + _Alignof(union R40)) abi_fail(-1, 40);
    calls += abi_t0();
    calls += abi_t1();
    calls += abi_t2();
    calls += abi_t3();
    calls += abi_t4();
    calls += abi_t5();
    calls += abi_t6();
    calls += abi_t7();
    if (abi_result() != 0) print_i64(abi_first());
    print_i64(abi_result());
    print_i64(calls);
    return 0;
}
