/* flags: --c-std=gnu17 */
/* Copies and fills of a constant size from 256 bytes up (the backends' loops
   of wide moves, aligning the destination to a cache line, and the parts
   overlapping at both ends; above 2 KiB the string instructions or the
   library's functions): every byte of the destination is written and none
   around it, from and to every alignment, onto the source itself, and from
   whole structures. Each line is the number of failed cases (0). */
#include "host.h"

static unsigned char src[4400], dst[4400];

static void reset(void) {
    for (int i = 0; i < 4400; i++) {
        src[i] = (unsigned char)(i * 7 + 3 + (i >> 8));
        dst[i] = 0xEE;
    }
}

/* 0 if dst[d..d+n] is src[s..s+n] and every other byte 0xEE */
static int bad_copy(int d, int s, int n) {
    for (int i = 0; i < 4400; i++) {
        int want = (i >= d && i < d + n) ? src[i - d + s] : 0xEE;
        if (dst[i] != want) return 1;
    }
    return 0;
}

/* 0 if dst[d..d+n] is all c and every other byte 0xEE */
static int bad_fill(int d, int n, int c) {
    for (int i = 0; i < 4400; i++) {
        int want = (i >= d && i < d + n) ? c : 0xEE;
        if (dst[i] != want) return 1;
    }
    return 0;
}

static const int offs[] = { 0, 1, 4, 8, 15, 16, 31, 32, 33, 48, 63, 64, 100 };
#define NOFF ((int)(sizeof offs / sizeof offs[0]))

#define COPIES(N)                                                   \
    static int copies_##N(void) {                                   \
        int bad = 0;                                                \
        for (int a = 0; a < NOFF; a++)                              \
            for (int b = 0; b < NOFF; b += 3) {                     \
                reset();                                            \
                __builtin_memcpy(dst + offs[a], src + offs[b], N);  \
                bad += bad_copy(offs[a], offs[b], N);               \
                reset();                                            \
                __builtin_memset(dst + offs[a], 0, N);              \
                bad += bad_fill(offs[a], N, 0);                     \
                reset();                                            \
                __builtin_memset(dst + offs[a], 0x5A, N);           \
                bad += bad_fill(offs[a], N, 0x5A);                  \
            }                                                       \
        return bad;                                                 \
    }
COPIES(257)
COPIES(272)
COPIES(320)
COPIES(449)
COPIES(511)
COPIES(1024)
COPIES(1604)
COPIES(2047)
COPIES(2048)
COPIES(2049)
COPIES(4100)

/* a memmove of a block onto itself, through pointers the compiler cannot
   tell apart */
static unsigned char *volatile vp;
static int onto_itself(void) {
    reset();
    unsigned char *p = vp;
    __builtin_memmove(p + 3, p + 3, 1604);
    for (int i = 0; i < 4400; i++)
        if (src[i] != (unsigned char)(i * 7 + 3 + (i >> 8))) return 1;
    return 0;
}

typedef struct { int n; unsigned d[400]; } big;  /* 1604 bytes */
typedef struct { char c; double x[40]; char e[3]; } odd;  /* 336 bytes */
static big G1, G2;
static odd O1, O2;

static int structs(void) {
    int bad = 0;
    big t;
    for (int i = 0; i < 400; i++) G1.d[i] = (unsigned)i * 2654435761u;
    G1.n = 77;
    t = G1;
    t.d[399] ^= 1;
    G2 = t;
    for (int i = 0; i < 399; i++) bad += G2.d[i] != (unsigned)i * 2654435761u;
    bad += G2.n != 77;
    bad += G2.d[399] != (399u * 2654435761u ^ 1);
    for (int i = 0; i < 40; i++) O1.x[i] = i * 0.5;
    O1.c = 'c';
    O1.e[2] = 'e';
    O2 = O1;
    for (int i = 0; i < 40; i++) bad += O2.x[i] != i * 0.5;
    bad += O2.c != 'c' || O2.e[2] != 'e';
    return bad;
}

int main(void) {
    vp = src;
    print_i64(copies_257() + copies_272() + copies_320() + copies_449() + copies_511());
    print_i64(copies_1024() + copies_1604() + copies_2047() + copies_2048());
    print_i64(copies_2049() + copies_4100());
    print_i64(onto_itself());
    print_i64(structs());
    return 0;
}

/* expect: 0 */
/* expect: 0 */
/* expect: 0 */
/* expect: 0 */
/* expect: 0 */
