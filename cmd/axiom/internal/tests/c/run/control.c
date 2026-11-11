/* Statements: if/else, while, do, for (with and without declarations),
   switch with fallthrough, nesting and default, break, continue, goto. */
#include "host.h"

static int classify(int x) {
    switch (x) {
    case 0:
        return 100;
    case 1:
    case 2:
        x += 10;
        /* fall through */
    case 3:
        x *= 2;
        break;
    case -1:
        return -100;
    default:
        x = -x;
    }
    return x;
}

static int nested(int a, int b) {
    int r = 0;
    switch (a) {
    case 1:
        switch (b) {
        case 1: r = 11; break;
        case 2: r = 12; break;
        }
        r += 1;
        break;
    case 2:
        r = 20;
    }
    return r;
}

static int chars(char c) {
    switch (c) {
    case 'a': return 1;
    case 'z': return 26;
    case '\n': return -1;
    }
    return 0;
}

int main(void) {
    int i, s = 0, n;
    for (i = -2; i < 6; i++) print_i64(classify(i));
    print_i64(nested(1, 1));
    print_i64(nested(1, 2));
    print_i64(nested(1, 3));
    print_i64(nested(2, 0));
    print_i64(nested(3, 0));
    print_i64(chars('a') + chars('z') + chars('\n') + chars('q'));
    for (i = 0; i < 20; i++) {
        if (i % 3 == 0) continue;
        if (i > 14) break;
        s += i;
    }
    print_i64(s);
    n = 0;
    while (n < 100) n = n * 2 + 1;
    print_i64(n);
    n = 0;
    do n++; while (n < 0);
    print_i64(n);
    s = 0;
    for (int k = 0, m = 10; k < m; k++, m--) s += k * m;
    print_i64(s);
    i = 0;
again:
    i++;
    if (i < 5) goto again;
    print_i64(i);
    goto skip;
    print_i64(999);
skip:
    s = 0;
    for (;;) {
        if (++s == 7) break;
    }
    print_i64(s);
    s = 0;
    for (i = 0; i < 4; i++) {
        int j = 0;
        while (1) {
            if (j == i) break;
            j++;
            s++;
        }
    }
    print_i64(s);
    if (s > 5) if (s > 100) print_i64(1); else print_i64(2);
    else print_i64(3);
    {
        int k = 5;
        do {
            if (k == 2) { k--; continue; }
            s += k;
            k--;
        } while (k > 0);
        print_i64(s);
    }
    switch (s) default: print_i64(-7);
    switch (s) { }
    return 0;
}

/* expect: 2 */
/* expect: -100 */
/* expect: 100 */
/* expect: 22 */
/* expect: 24 */
/* expect: 6 */
/* expect: -4 */
/* expect: -5 */
/* expect: 12 */
/* expect: 13 */
/* expect: 1 */
/* expect: 20 */
/* expect: 0 */
/* expect: 26 */
/* expect: 75 */
/* expect: 127 */
/* expect: 1 */
/* expect: 70 */
/* expect: 5 */
/* expect: 7 */
/* expect: 6 */
/* expect: 2 */
/* expect: 19 */
/* expect: -7 */
