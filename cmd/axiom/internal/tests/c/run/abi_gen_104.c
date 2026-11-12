/* abi_gen_104.c - written by gen_c_abi.py 104 8: calls of the
   functions of abi_gen_104_callee.c, directly, through pointers and with
   variadic arguments; prints the failures and the calls made. */
/* flags: --c-std=gnu23 */
/* units: abi/abi_gen_104_callee.c */
#include "host.h"
#include "abi/abi_gen_104.h"

int abi_result(void);
int abi_first(void);
unsigned long long abi_layout(int k);

static struct R0 (*volatile abi_p0)(void) = abi_f0;
static void (*volatile abi_p1)(long long a0, long long a1) = abi_f1;
static struct R1 (*volatile abi_p2)(float a0, float a1, struct R2 a2, struct R4 a3, struct R5 a4, struct R6 a5, union R12 a6, struct R13 a7, struct R14 a8, _BitInt(128) a9, signed char a10, double a11, ...) = abi_f2;
static struct R18 (*volatile abi_p3)(unsigned long long a0, ...) = abi_f3;
static _Bool (*volatile abi_p4)(union R21 a0, struct R26 a1) = abi_f4;
static struct R27 (*volatile abi_p5)(struct R28 a0) = abi_f5;
static struct R29 (*volatile abi_p6)(union R30 a0, _BitInt(65) a1, struct R31 a2, struct R33 a3) = abi_f6;
static void (*volatile abi_p7)(unsigned char a0, unsigned __int128 a1, struct R34 a2, struct R35 a3, struct R36 a4, struct R37 a5, unsigned long long a6, struct R38 a7, ...) = abi_f7;

static int abi_t0(void) {
    int bad = 0;
    {
        struct R0 r = abi_f0();
        if (!(check_R0(&r, 99u))) bad = 1;
    }
    {
        struct R0 r = abi_p0();
        if (!(check_R0(&r, 99u))) bad = 1;
    }
    if (bad) abi_fail(0, 99);
    return 2;
}

static int abi_t1(void) {
    long long a0 = (long long)abi_mix(100u, 0u);
    long long a1 = (long long)abi_mix(101u, 0u);
    int bad = 0;
    abi_f1(a0, a1);
    abi_p1(a0, a1);
    if (bad) abi_fail(1, 99);
    return 2;
}

static int abi_t2(void) {
    float a0 = (float)((double)(long long)(abi_mix(200u, 0u) % 4001u) / 8.0 - 250.0);
    float a1 = (float)((double)(long long)(abi_mix(201u, 0u) % 4001u) / 8.0 - 250.0);
    struct R2 a2; fill_R2(&a2, 202u);
    struct R4 a3; fill_R4(&a3, 203u);
    struct R5 a4; fill_R5(&a4, 204u);
    struct R6 a5; fill_R6(&a5, 205u);
    union R12 a6; fill_R12(&a6, 206u);
    struct R13 a7; fill_R13(&a7, 207u);
    struct R14 a8; fill_R14(&a8, 208u);
    _BitInt(128) a9 = (_BitInt(128))((unsigned _BitInt(128))abi_mix(209u, 0u) ^ ((unsigned _BitInt(128))abi_mix(209u, 0u + 2000u) << 64));
    signed char a10 = (signed char)((long long)(abi_mix(210u, 0u) % 256u) - 128);
    double a11 = (double)((double)(long long)(abi_mix(211u, 0u) % 4001u) / 8.0 - 250.0);
    union R16 v0; fill_R16(&v0, 250u);
    __int128 v1 = (__int128)((unsigned __int128)abi_mix(251u, 0u) ^ ((unsigned __int128)abi_mix(251u, 0u + 2000u) << 64));
    int bad = 0;
    {
        struct R1 r = abi_f2(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, v0, v1);
        if (!(check_R1(&r, 299u))) bad = 1;
    }
    {
        struct R1 r = abi_p2(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, v0, v1);
        if (!(check_R1(&r, 299u))) bad = 1;
    }
    if (bad) abi_fail(2, 99);
    return 2;
}

static int abi_t3(void) {
    unsigned long long a0 = (unsigned long long)abi_mix(300u, 0u);
    unsigned _BitInt(256) v0 = (unsigned _BitInt(256))((unsigned _BitInt(256))abi_mix(350u, 0u) ^ ((unsigned _BitInt(256))abi_mix(350u, 0u + 2000u) << 64) ^ ((unsigned _BitInt(256))abi_mix(350u, 0u + 3000u) << 192));
    char * v1 = (abi_anchor + (abi_mix(351u, 0u) % 8u));
    struct R19 v2; fill_R19(&v2, 352u);
    long long v3 = (long long)abi_mix(353u, 0u);
    struct R20 v4; fill_R20(&v4, 354u);
    int v5 = (int)((long long)(abi_mix(355u, 0u) % 4294967296u) - 2147483648);
    int bad = 0;
    {
        struct R18 r = abi_f3(a0, v0, v1, v2, v3, v4, v5);
        if (!(check_R18(&r, 399u))) bad = 1;
    }
    {
        struct R18 r = abi_p3(a0, v0, v1, v2, v3, v4, v5);
        if (!(check_R18(&r, 399u))) bad = 1;
    }
    if (bad) abi_fail(3, 99);
    return 2;
}

static int abi_t4(void) {
    union R21 a0; fill_R21(&a0, 400u);
    struct R26 a1; fill_R26(&a1, 401u);
    int bad = 0;
    {
        _Bool r = abi_f4(a0, a1);
        if (!(r == (_Bool)(abi_mix(499u, 0u) & 1u))) bad = 1;
    }
    {
        _Bool r = abi_p4(a0, a1);
        if (!(r == (_Bool)(abi_mix(499u, 0u) & 1u))) bad = 1;
    }
    if (bad) abi_fail(4, 99);
    return 2;
}

static int abi_t5(void) {
    struct R28 a0; fill_R28(&a0, 500u);
    int bad = 0;
    {
        struct R27 r = abi_f5(a0);
        if (!(check_R27(&r, 599u))) bad = 1;
    }
    {
        struct R27 r = abi_p5(a0);
        if (!(check_R27(&r, 599u))) bad = 1;
    }
    if (bad) abi_fail(5, 99);
    return 2;
}

static int abi_t6(void) {
    union R30 a0; fill_R30(&a0, 600u);
    _BitInt(65) a1 = (_BitInt(65))((unsigned _BitInt(65))abi_mix(601u, 0u) ^ ((unsigned _BitInt(65))abi_mix(601u, 0u + 2000u) << 64));
    struct R31 a2; fill_R31(&a2, 602u);
    struct R33 a3; fill_R33(&a3, 603u);
    int bad = 0;
    {
        struct R29 r = abi_f6(a0, a1, a2, a3);
        if (!(check_R29(&r, 699u))) bad = 1;
    }
    {
        struct R29 r = abi_p6(a0, a1, a2, a3);
        if (!(check_R29(&r, 699u))) bad = 1;
    }
    if (bad) abi_fail(6, 99);
    return 2;
}

static int abi_t7(void) {
    unsigned char a0 = (unsigned char)(abi_mix(700u, 0u) % 256u);
    unsigned __int128 a1 = (unsigned __int128)((unsigned __int128)abi_mix(701u, 0u) ^ ((unsigned __int128)abi_mix(701u, 0u + 2000u) << 64));
    struct R34 a2; fill_R34(&a2, 702u);
    struct R35 a3; fill_R35(&a3, 703u);
    struct R36 a4; fill_R36(&a4, 704u);
    struct R37 a5; fill_R37(&a5, 705u);
    unsigned long long a6 = (unsigned long long)abi_mix(706u, 0u);
    struct R38 a7; fill_R38(&a7, 707u);
    struct R39 v0; fill_R39(&v0, 750u);
    struct R40 v1; fill_R40(&v1, 751u);
    union R41 v2; fill_R41(&v2, 752u);
    int bad = 0;
    abi_f7(a0, a1, a2, a3, a4, a5, a6, a7, v0, v1, v2);
    abi_p7(a0, a1, a2, a3, a4, a5, a6, a7, v0, v1, v2);
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
    if (abi_layout(8) != sizeof(union R8) * 1000 + _Alignof(union R8)) abi_fail(-1, 8);
    if (abi_layout(9) != sizeof(struct R9) * 1000 + _Alignof(struct R9)) abi_fail(-1, 9);
    if (abi_layout(10) != sizeof(struct R10) * 1000 + _Alignof(struct R10)) abi_fail(-1, 10);
    if (abi_layout(11) != sizeof(struct R11) * 1000 + _Alignof(struct R11)) abi_fail(-1, 11);
    if (abi_layout(12) != sizeof(union R12) * 1000 + _Alignof(union R12)) abi_fail(-1, 12);
    if (abi_layout(13) != sizeof(struct R13) * 1000 + _Alignof(struct R13)) abi_fail(-1, 13);
    if (abi_layout(14) != sizeof(struct R14) * 1000 + _Alignof(struct R14)) abi_fail(-1, 14);
    if (abi_layout(15) != sizeof(struct R15) * 1000 + _Alignof(struct R15)) abi_fail(-1, 15);
    if (abi_layout(16) != sizeof(union R16) * 1000 + _Alignof(union R16)) abi_fail(-1, 16);
    if (abi_layout(17) != sizeof(struct R17) * 1000 + _Alignof(struct R17)) abi_fail(-1, 17);
    if (abi_layout(18) != sizeof(struct R18) * 1000 + _Alignof(struct R18)) abi_fail(-1, 18);
    if (abi_layout(19) != sizeof(struct R19) * 1000 + _Alignof(struct R19)) abi_fail(-1, 19);
    if (abi_layout(20) != sizeof(struct R20) * 1000 + _Alignof(struct R20)) abi_fail(-1, 20);
    if (abi_layout(21) != sizeof(union R21) * 1000 + _Alignof(union R21)) abi_fail(-1, 21);
    if (abi_layout(22) != sizeof(struct R22) * 1000 + _Alignof(struct R22)) abi_fail(-1, 22);
    if (abi_layout(23) != sizeof(struct R23) * 1000 + _Alignof(struct R23)) abi_fail(-1, 23);
    if (abi_layout(24) != sizeof(struct R24) * 1000 + _Alignof(struct R24)) abi_fail(-1, 24);
    if (abi_layout(25) != sizeof(struct R25) * 1000 + _Alignof(struct R25)) abi_fail(-1, 25);
    if (abi_layout(26) != sizeof(struct R26) * 1000 + _Alignof(struct R26)) abi_fail(-1, 26);
    if (abi_layout(27) != sizeof(struct R27) * 1000 + _Alignof(struct R27)) abi_fail(-1, 27);
    if (abi_layout(28) != sizeof(struct R28) * 1000 + _Alignof(struct R28)) abi_fail(-1, 28);
    if (abi_layout(29) != sizeof(struct R29) * 1000 + _Alignof(struct R29)) abi_fail(-1, 29);
    if (abi_layout(30) != sizeof(union R30) * 1000 + _Alignof(union R30)) abi_fail(-1, 30);
    if (abi_layout(31) != sizeof(struct R31) * 1000 + _Alignof(struct R31)) abi_fail(-1, 31);
    if (abi_layout(32) != sizeof(struct R32) * 1000 + _Alignof(struct R32)) abi_fail(-1, 32);
    if (abi_layout(33) != sizeof(struct R33) * 1000 + _Alignof(struct R33)) abi_fail(-1, 33);
    if (abi_layout(34) != sizeof(struct R34) * 1000 + _Alignof(struct R34)) abi_fail(-1, 34);
    if (abi_layout(35) != sizeof(struct R35) * 1000 + _Alignof(struct R35)) abi_fail(-1, 35);
    if (abi_layout(36) != sizeof(struct R36) * 1000 + _Alignof(struct R36)) abi_fail(-1, 36);
    if (abi_layout(37) != sizeof(struct R37) * 1000 + _Alignof(struct R37)) abi_fail(-1, 37);
    if (abi_layout(38) != sizeof(struct R38) * 1000 + _Alignof(struct R38)) abi_fail(-1, 38);
    if (abi_layout(39) != sizeof(struct R39) * 1000 + _Alignof(struct R39)) abi_fail(-1, 39);
    if (abi_layout(40) != sizeof(struct R40) * 1000 + _Alignof(struct R40)) abi_fail(-1, 40);
    if (abi_layout(41) != sizeof(union R41) * 1000 + _Alignof(union R41)) abi_fail(-1, 41);
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
/* expect: 0 */
/* expect: 16 */
