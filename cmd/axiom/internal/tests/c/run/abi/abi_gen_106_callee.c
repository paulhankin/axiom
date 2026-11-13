/* abi_gen_106_callee.c - written by gen_c_abi.py 105 8: the functions
   that abi_gen_106.c calls, which check their arguments. */
#include "abi_gen_106.h"

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
        int v = va_arg(ap, int);
        if (!(v == (int)((long long)(abi_mix(50u, 0u) % 4294967296u) - 2147483648))) abi_fail(0, 200);
    }
    if (from <= 1) {
        struct R5 v = va_arg(ap, struct R5);
        if (!(check_R5(&v, 51u))) abi_fail(0, 201);
    }
    if (from <= 2) {
        struct R10 v = va_arg(ap, struct R10);
        if (!(check_R10(&v, 52u))) abi_fail(0, 202);
    }
    if (from <= 3) {
        struct R12 v = va_arg(ap, struct R12);
        if (!(check_R12(&v, 53u))) abi_fail(0, 203);
    }
    if (from <= 4) {
        struct R13 v = va_arg(ap, struct R13);
        if (!(check_R13(&v, 54u))) abi_fail(0, 204);
    }
    if (from <= 5) {
        struct R14 v = va_arg(ap, struct R14);
        if (!(check_R14(&v, 55u))) abi_fail(0, 205);
    }
}
struct R0 abi_f0(struct R3 a0, ...) {
    if (!(check_R3(&a0, 0u))) abi_fail(0, 0);
    va_list ap, aq, ar;
    va_start(ap, a0);
    {
        int v = va_arg(ap, int);
        if (!(v == (int)((long long)(abi_mix(50u, 0u) % 4294967296u) - 2147483648))) abi_fail(0, 100);
    }
    {
        struct R5 v = va_arg(ap, struct R5);
        if (!(check_R5(&v, 51u))) abi_fail(0, 101);
    }
    {
        struct R10 v = va_arg(ap, struct R10);
        if (!(check_R10(&v, 52u))) abi_fail(0, 102);
    }
    {
        struct R12 v = va_arg(ap, struct R12);
        if (!(check_R12(&v, 53u))) abi_fail(0, 103);
    }
    va_copy(aq, ap);
    {
        struct R13 v = va_arg(ap, struct R13);
        if (!(check_R13(&v, 54u))) abi_fail(0, 104);
    }
    {
        struct R14 v = va_arg(ap, struct R14);
        if (!(check_R14(&v, 55u))) abi_fail(0, 105);
    }
    va_end(ap);
    va_copy(ar, aq);
    abi_v0(aq, 4);
    va_end(aq);
    abi_v0(ar, 4);
    va_end(ar);
    struct R0 r; fill_R0(&r, 99u);
    return r;
}

static void abi_v1(va_list ap, int from) {
    if (from <= 0) {
        struct R18 v = va_arg(ap, struct R18);
        if (!(check_R18(&v, 150u))) abi_fail(1, 200);
    }
    if (from <= 1) {
        long long v = va_arg(ap, long long);
        if (!(v == (long long)abi_mix(151u, 0u))) abi_fail(1, 201);
    }
    if (from <= 2) {
        struct R24 v = va_arg(ap, struct R24);
        if (!(check_R24(&v, 152u))) abi_fail(1, 202);
    }
    if (from <= 3) {
        _Float128 v = va_arg(ap, _Float128);
        if (!(v == ((_Float128)(long long)(abi_mix(153u, 0u) % 4001u) / 8 - 250 + (_Float128)(abi_mix(153u, 0u) % 7u) * 0x1p-100f128))) abi_fail(1, 203);
    }
    if (from <= 4) {
        struct R25 v = va_arg(ap, struct R25);
        if (!(check_R25(&v, 154u))) abi_fail(1, 204);
    }
    if (from <= 5) {
        struct R27 v = va_arg(ap, struct R27);
        if (!(check_R27(&v, 155u))) abi_fail(1, 205);
    }
    if (from <= 6) {
        struct R28 v = va_arg(ap, struct R28);
        if (!(check_R28(&v, 156u))) abi_fail(1, 206);
    }
}
unsigned long long abi_f1(struct R15 a0, struct R16 a1, union R17 a2, ...) {
    if (!(check_R15(&a0, 100u))) abi_fail(1, 0);
    if (!(check_R16(&a1, 101u))) abi_fail(1, 1);
    if (!(check_R17(&a2, 102u))) abi_fail(1, 2);
    va_list ap, aq, ar;
    va_start(ap, a2);
    {
        struct R18 v = va_arg(ap, struct R18);
        if (!(check_R18(&v, 150u))) abi_fail(1, 100);
    }
    {
        long long v = va_arg(ap, long long);
        if (!(v == (long long)abi_mix(151u, 0u))) abi_fail(1, 101);
    }
    {
        struct R24 v = va_arg(ap, struct R24);
        if (!(check_R24(&v, 152u))) abi_fail(1, 102);
    }
    {
        _Float128 v = va_arg(ap, _Float128);
        if (!(v == ((_Float128)(long long)(abi_mix(153u, 0u) % 4001u) / 8 - 250 + (_Float128)(abi_mix(153u, 0u) % 7u) * 0x1p-100f128))) abi_fail(1, 103);
    }
    {
        struct R25 v = va_arg(ap, struct R25);
        if (!(check_R25(&v, 154u))) abi_fail(1, 104);
    }
    va_copy(aq, ap);
    {
        struct R27 v = va_arg(ap, struct R27);
        if (!(check_R27(&v, 155u))) abi_fail(1, 105);
    }
    {
        struct R28 v = va_arg(ap, struct R28);
        if (!(check_R28(&v, 156u))) abi_fail(1, 106);
    }
    va_end(ap);
    va_copy(ar, aq);
    abi_v1(aq, 5);
    va_end(aq);
    abi_v1(ar, 5);
    va_end(ar);
    unsigned long long r = (unsigned long long)abi_mix(199u, 0u);
    return r;
}

unsigned long long abi_f2(abi_v8uc a0, unsigned char a1, abi_v2ll a2) {
    if (!(check_abi_v8uc(&a0, 200u))) abi_fail(2, 0);
    if (!(a1 == (unsigned char)(abi_mix(201u, 0u) % 256u))) abi_fail(2, 1);
    if (!(check_abi_v2ll(&a2, 202u))) abi_fail(2, 2);
    unsigned long long r = (unsigned long long)abi_mix(299u, 0u);
    return r;
}

static void abi_v3(va_list ap, int from) {
    if (from <= 0) {
        union R32 v = va_arg(ap, union R32);
        if (!(check_R32(&v, 350u))) abi_fail(3, 200);
    }
    if (from <= 1) {
        struct R34 v = va_arg(ap, struct R34);
        if (!(check_R34(&v, 351u))) abi_fail(3, 201);
    }
    if (from <= 2) {
        struct R36 v = va_arg(ap, struct R36);
        if (!(check_R36(&v, 352u))) abi_fail(3, 202);
    }
    if (from <= 3) {
        struct R37 v = va_arg(ap, struct R37);
        if (!(check_R37(&v, 353u))) abi_fail(3, 203);
    }
    if (from <= 4) {
        struct R39 v = va_arg(ap, struct R39);
        if (!(check_R39(&v, 354u))) abi_fail(3, 204);
    }
    if (from <= 5) {
        union R40 v = va_arg(ap, union R40);
        if (!(check_R40(&v, 355u))) abi_fail(3, 205);
    }
}
void abi_f3(struct R31 a0, ...) {
    if (!(check_R31(&a0, 300u))) abi_fail(3, 0);
    va_list ap, aq, ar;
    va_start(ap, a0);
    {
        union R32 v = va_arg(ap, union R32);
        if (!(check_R32(&v, 350u))) abi_fail(3, 100);
    }
    {
        struct R34 v = va_arg(ap, struct R34);
        if (!(check_R34(&v, 351u))) abi_fail(3, 101);
    }
    {
        struct R36 v = va_arg(ap, struct R36);
        if (!(check_R36(&v, 352u))) abi_fail(3, 102);
    }
    {
        struct R37 v = va_arg(ap, struct R37);
        if (!(check_R37(&v, 353u))) abi_fail(3, 103);
    }
    va_copy(aq, ap);
    {
        struct R39 v = va_arg(ap, struct R39);
        if (!(check_R39(&v, 354u))) abi_fail(3, 104);
    }
    {
        union R40 v = va_arg(ap, union R40);
        if (!(check_R40(&v, 355u))) abi_fail(3, 105);
    }
    va_end(ap);
    va_copy(ar, aq);
    abi_v3(aq, 4);
    va_end(aq);
    abi_v3(ar, 4);
    va_end(ar);
}

struct R42 abi_f4(int a0, abi_v2d a1, _Float16 a2, int a3, float a4, _Float128 a5, abi_v2d a6, struct R43 a7, _Float16 a8, abi_v4c a9, struct R44 a10, struct R47 a11, struct R48 a12, struct R49 a13, float a14) {
    if (!(a0 == (int)((long long)(abi_mix(400u, 0u) % 4294967296u) - 2147483648))) abi_fail(4, 0);
    if (!(check_abi_v2d(&a1, 401u))) abi_fail(4, 1);
    if (!(a2 == (_Float16)((double)(long long)(abi_mix(402u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(4, 2);
    if (!(a3 == (int)((long long)(abi_mix(403u, 0u) % 4294967296u) - 2147483648))) abi_fail(4, 3);
    if (!(a4 == (float)((double)(long long)(abi_mix(404u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(4, 4);
    if (!(a5 == ((_Float128)(long long)(abi_mix(405u, 0u) % 4001u) / 8 - 250 + (_Float128)(abi_mix(405u, 0u) % 7u) * 0x1p-100f128))) abi_fail(4, 5);
    if (!(check_abi_v2d(&a6, 406u))) abi_fail(4, 6);
    if (!(check_R43(&a7, 407u))) abi_fail(4, 7);
    if (!(a8 == (_Float16)((double)(long long)(abi_mix(408u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(4, 8);
    if (!(check_abi_v4c(&a9, 409u))) abi_fail(4, 9);
    if (!(check_R44(&a10, 410u))) abi_fail(4, 10);
    if (!(check_R47(&a11, 411u))) abi_fail(4, 11);
    if (!(check_R48(&a12, 412u))) abi_fail(4, 12);
    if (!(check_R49(&a13, 413u))) abi_fail(4, 13);
    if (!(a14 == (float)((double)(long long)(abi_mix(414u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(4, 14);
    struct R42 r; fill_R42(&r, 499u);
    return r;
}

static void abi_v5(va_list ap, int from) {
    if (from <= 0) {
        abi_v2f v = va_arg(ap, abi_v2f);
        if (!(check_abi_v2f(&v, 550u))) abi_fail(5, 200);
    }
    if (from <= 1) {
        union R62 v = va_arg(ap, union R62);
        if (!(check_R62(&v, 551u))) abi_fail(5, 201);
    }
    if (from <= 2) {
        struct R66 v = va_arg(ap, struct R66);
        if (!(check_R66(&v, 552u))) abi_fail(5, 202);
    }
    if (from <= 3) {
        struct R71 v = va_arg(ap, struct R71);
        if (!(check_R71(&v, 553u))) abi_fail(5, 203);
    }
    if (from <= 4) {
        unsigned long long v = va_arg(ap, unsigned long long);
        if (!(v == (unsigned long long)abi_mix(554u, 0u))) abi_fail(5, 204);
    }
    if (from <= 5) {
        struct R73 v = va_arg(ap, struct R73);
        if (!(check_R73(&v, 555u))) abi_fail(5, 205);
    }
}
struct R50 abi_f5(char * a0, struct R59 a1, struct R60 a2, struct R61 a3, ...) {
    if (!(a0 == (abi_anchor + (abi_mix(500u, 0u) % 8u)))) abi_fail(5, 0);
    if (!(check_R59(&a1, 501u))) abi_fail(5, 1);
    if (!(check_R60(&a2, 502u))) abi_fail(5, 2);
    if (!(check_R61(&a3, 503u))) abi_fail(5, 3);
    va_list ap, aq, ar;
    va_start(ap, a3);
    {
        abi_v2f v = va_arg(ap, abi_v2f);
        if (!(check_abi_v2f(&v, 550u))) abi_fail(5, 100);
    }
    {
        union R62 v = va_arg(ap, union R62);
        if (!(check_R62(&v, 551u))) abi_fail(5, 101);
    }
    {
        struct R66 v = va_arg(ap, struct R66);
        if (!(check_R66(&v, 552u))) abi_fail(5, 102);
    }
    {
        struct R71 v = va_arg(ap, struct R71);
        if (!(check_R71(&v, 553u))) abi_fail(5, 103);
    }
    {
        unsigned long long v = va_arg(ap, unsigned long long);
        if (!(v == (unsigned long long)abi_mix(554u, 0u))) abi_fail(5, 104);
    }
    va_copy(aq, ap);
    {
        struct R73 v = va_arg(ap, struct R73);
        if (!(check_R73(&v, 555u))) abi_fail(5, 105);
    }
    va_end(ap);
    va_copy(ar, aq);
    abi_v5(aq, 5);
    va_end(aq);
    abi_v5(ar, 5);
    va_end(ar);
    struct R50 r; fill_R50(&r, 599u);
    return r;
}

struct R74 abi_f6(struct R76 a0, abi_v4f a1, struct R77 a2, struct R79 a3) {
    if (!(check_R76(&a0, 600u))) abi_fail(6, 0);
    if (!(check_abi_v4f(&a1, 601u))) abi_fail(6, 1);
    if (!(check_R77(&a2, 602u))) abi_fail(6, 2);
    if (!(check_R79(&a3, 603u))) abi_fail(6, 3);
    struct R74 r; fill_R74(&r, 699u);
    return r;
}

void abi_f7(double a0, int a1, struct R80 a2) {
    if (!(a0 == (double)((double)(long long)(abi_mix(700u, 0u) % 4001u) / 8.0 - 250.0))) abi_fail(7, 0);
    if (!(a1 == (int)((long long)(abi_mix(701u, 0u) % 4294967296u) - 2147483648))) abi_fail(7, 1);
    if (!(check_R80(&a2, 702u))) abi_fail(7, 2);
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
    case 6: return sizeof(union R6) * 1000 + _Alignof(union R6);
    case 7: return sizeof(struct R7) * 1000 + _Alignof(struct R7);
    case 8: return sizeof(struct R8) * 1000 + _Alignof(struct R8);
    case 9: return sizeof(struct R9) * 1000 + _Alignof(struct R9);
    case 10: return sizeof(struct R10) * 1000 + _Alignof(struct R10);
    case 11: return sizeof(struct R11) * 1000 + _Alignof(struct R11);
    case 12: return sizeof(struct R12) * 1000 + _Alignof(struct R12);
    case 13: return sizeof(struct R13) * 1000 + _Alignof(struct R13);
    case 14: return sizeof(struct R14) * 1000 + _Alignof(struct R14);
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
    case 30: return sizeof(union R30) * 1000 + _Alignof(union R30);
    case 31: return sizeof(struct R31) * 1000 + _Alignof(struct R31);
    case 32: return sizeof(union R32) * 1000 + _Alignof(union R32);
    case 33: return sizeof(struct R33) * 1000 + _Alignof(struct R33);
    case 34: return sizeof(struct R34) * 1000 + _Alignof(struct R34);
    case 35: return sizeof(struct R35) * 1000 + _Alignof(struct R35);
    case 36: return sizeof(struct R36) * 1000 + _Alignof(struct R36);
    case 37: return sizeof(struct R37) * 1000 + _Alignof(struct R37);
    case 38: return sizeof(struct R38) * 1000 + _Alignof(struct R38);
    case 39: return sizeof(struct R39) * 1000 + _Alignof(struct R39);
    case 40: return sizeof(union R40) * 1000 + _Alignof(union R40);
    case 41: return sizeof(struct R41) * 1000 + _Alignof(struct R41);
    case 42: return sizeof(struct R42) * 1000 + _Alignof(struct R42);
    case 43: return sizeof(struct R43) * 1000 + _Alignof(struct R43);
    case 44: return sizeof(struct R44) * 1000 + _Alignof(struct R44);
    case 45: return sizeof(union R45) * 1000 + _Alignof(union R45);
    case 46: return sizeof(struct R46) * 1000 + _Alignof(struct R46);
    case 47: return sizeof(struct R47) * 1000 + _Alignof(struct R47);
    case 48: return sizeof(struct R48) * 1000 + _Alignof(struct R48);
    case 49: return sizeof(struct R49) * 1000 + _Alignof(struct R49);
    case 50: return sizeof(struct R50) * 1000 + _Alignof(struct R50);
    case 51: return sizeof(struct R51) * 1000 + _Alignof(struct R51);
    case 52: return sizeof(union R52) * 1000 + _Alignof(union R52);
    case 53: return sizeof(struct R53) * 1000 + _Alignof(struct R53);
    case 54: return sizeof(struct R54) * 1000 + _Alignof(struct R54);
    case 55: return sizeof(struct R55) * 1000 + _Alignof(struct R55);
    case 56: return sizeof(struct R56) * 1000 + _Alignof(struct R56);
    case 57: return sizeof(struct R57) * 1000 + _Alignof(struct R57);
    case 58: return sizeof(union R58) * 1000 + _Alignof(union R58);
    case 59: return sizeof(struct R59) * 1000 + _Alignof(struct R59);
    case 60: return sizeof(struct R60) * 1000 + _Alignof(struct R60);
    case 61: return sizeof(struct R61) * 1000 + _Alignof(struct R61);
    case 62: return sizeof(union R62) * 1000 + _Alignof(union R62);
    case 63: return sizeof(struct R63) * 1000 + _Alignof(struct R63);
    case 64: return sizeof(union R64) * 1000 + _Alignof(union R64);
    case 65: return sizeof(union R65) * 1000 + _Alignof(union R65);
    case 66: return sizeof(struct R66) * 1000 + _Alignof(struct R66);
    case 67: return sizeof(struct R67) * 1000 + _Alignof(struct R67);
    case 68: return sizeof(struct R68) * 1000 + _Alignof(struct R68);
    case 69: return sizeof(struct R69) * 1000 + _Alignof(struct R69);
    case 70: return sizeof(struct R70) * 1000 + _Alignof(struct R70);
    case 71: return sizeof(struct R71) * 1000 + _Alignof(struct R71);
    case 72: return sizeof(struct R72) * 1000 + _Alignof(struct R72);
    case 73: return sizeof(struct R73) * 1000 + _Alignof(struct R73);
    case 74: return sizeof(struct R74) * 1000 + _Alignof(struct R74);
    case 75: return sizeof(struct R75) * 1000 + _Alignof(struct R75);
    case 76: return sizeof(struct R76) * 1000 + _Alignof(struct R76);
    case 77: return sizeof(struct R77) * 1000 + _Alignof(struct R77);
    case 78: return sizeof(struct R78) * 1000 + _Alignof(struct R78);
    case 79: return sizeof(struct R79) * 1000 + _Alignof(struct R79);
    case 80: return sizeof(struct R80) * 1000 + _Alignof(struct R80);
    }
    return 0;
}
