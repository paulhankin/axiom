/* Scopes and lifetimes in statements: a block-scope function declaration and
   an object hiding it, an object initialized again each time a backward goto
   reaches its declaration, a goto past an initialization, the scopes of for
   statements and their bodies, tags and enumeration constants in nested
   blocks, empty and null statements, break and continue in nested loops
   and switches. */
/* flags: --c-std=c99 */
#include "host.h"
int twice(int x) { return 2 * x; }
static int shadow_fn(void) {
    int r = twice(2);
    {
        int twice(int);  /* a block-scope declaration of the same function */
        r += twice(3);
    }
    {
        int twice = 100;  /* an object hiding it */
        r += twice;
    }
    return r;
}
static int lifetime(void) {
    int n = 0, sum = 0;
again:
    {
        int x = 5;  /* initialized each time its declaration is reached */
        sum += x;
        x = 99;
        if (++n < 3) goto again;
    }
    return sum;
}
static int skip_init(int k) {
    int r = 0;
    if (k) goto inside;  /* past an initialization: allowed (not variably modified) */
    {
        int y = 7;
        r = y;
    inside:
        y = 3;
        r += y;
    }
    return r;
}
static int scopes(void) {
    int i = 10, s = 0;
    for (int i = 0; i < 3; i++) s += i;
    s += i;
    for (int i = 0, j = 5; i < j; i++, j--) {
        int i = 100;  /* the body is a block inside the for statement's scope */
        s += i + j;
    }
    {
        struct T { int a; };
        struct T t = { 4 };
        s += t.a;
        {
            struct T { char c[3]; };
            s += (int)sizeof(struct T);
        }
        s += (int)sizeof(struct T);
    }
    enum { K = 1 };
    {
        enum { K = 50 };
        s += K;
    }
    s += K;
    return s;
}
static int empty_and_null(int x) {
    int r = 0;
    for (;;) break;
    while (x-- > 0) ;
    if (r) ; else r = 1;
    do ; while (0);
    {}
    ;;
    switch (r) ;
    switch (r) { }
    switch (r) case 1: r += 10;
    return r + x;
}
static int deep_break(void) {
    int s = 0;
    for (int i = 0; i < 5; i++) {
        switch (i) {
        case 1:
            continue;
        case 3:
            for (;;) {
                while (1) {
                    s += 1000;
                    break;
                }
                break;
            }
            break;
        default:
            do {
                s += i;
                if (i == 4) break;
            } while (0);
        }
        s += 10;
    }
    return s;
}
int main(void) {
    print_i64(shadow_fn());
    print_i64(lifetime());
    print_i64(skip_init(0) * 10 + skip_init(1));
    print_i64(scopes());
    print_i64(empty_and_null(3));
    print_i64(deep_break());
    return 0;
}

/* expect: 110 */
/* expect: 15 */
/* expect: 103 */
/* expect: 387 */
/* expect: 10 */
/* expect: 1046 */
