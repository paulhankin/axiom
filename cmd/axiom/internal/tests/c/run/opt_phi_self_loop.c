/* flags: --c-std=c99 -O2 */
/* Found by differential testing: the optimizer's sinking of an operation
   common to a phi's inputs moved one defined in the phi's own block (a loop
   of one block) to the block's start, and crashed. The functions are not
   called; they only have to compile. */
#include "host.h"

struct S0 { char f0; double f3; };
struct S2 { float f3; };
char g0 = (signed)0;
short g1;
unsigned g2;
int g3;
long ga1[] = { 1ULL };
struct S0 gsa0[] = { { (char)0 } };
struct S2 gsa2[] = { { (9223372036854775807LL) } };
enum { E1 = 793 } f0(int depth, int p1, short p2) {
    int l0 = (1);
    char l1 = (char)255;
    int i2;
    switch ((int)(((unsigned)(char)41))) {
    case 3:
        p2 |= p2;
        {
            int c2;
        L2:
            p2 = ((unsigned)l0);
            if (3) goto L2;
        }
        for (;;) {
            switch ((int)(((long)((long)(((int)((unsigned)gsa0[2].f0 + (unsigned)gsa0[3].f3)) | (l0))))))
                switch ((int)(((unsigned)((signed)p1)))) {
                }
        }
    }
}
char f1(struct S0 p2, long p3, short p8) {
    long l0 = (1);
    char l1 = (char)3;
    long l3 = (1);
    int i1 = 0;
    int i2 = 0;
    if ((((ga1[(((unsigned)((unsigned)(((((char)((int)0 | ((char)2 == (1))) ? (char)2 : (char)(int)E1)) &
                                         ((((int)E1 == (1)) ? 2 : (int)(char)(int)E1)) < 2147483648.0)
                                            ? (int)(((char)3e38f) - (((int)0 | ((char)2 == (1))) ? (char)2 : (char)(int)E1))
                                            : 0))))] -
          (long)l1)))) {
        if ((char)0) {
        }
    }
    for (;; i2++) {
        for (;; i1++) {
            switch ((int)(((long)((short)((1 ? p3 : ga1[(((g2)) % 16u)]), ((long)((unsigned)(long)l3))))))) {
            }
        }
        switch ((int)(((long)p8))) {
        }
    }
    {
        if ((l0 ? ((_Bool)((((int)g0 == 0 | ((int)p8 == (1))) ? (int)p8 : (int)gsa2[0].f3))) : ((_Bool)((((short)g3)))))) {
        }
    }
}

int main(void) {
    print_i64(sizeof gsa0 + g3);
    return 0;
}

/* expect: 16 */
