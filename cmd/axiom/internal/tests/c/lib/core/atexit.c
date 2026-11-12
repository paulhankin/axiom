/* atexit and exit (C17 7.22.4.2, 7.22.4.4): returning from main is exit
   with its value; the handlers run in the reverse order of their
   registration, 40 of them (more than the 32 the standard asks for); one
   registered by a handler runs next. */
/* expect-status: 9 */
#include <stdlib.h>
#include "print.h"

static int order[64], n;
static void (*fs[40])(void);

#define H(k) static void h##k(void) { order[n++] = k; }
H(0) H(1) H(2) H(3) H(4) H(5) H(6) H(7) H(8) H(9) H(10) H(11) H(12) H(13) H(14) H(15) H(16) H(17) H(18) H(19)
H(20) H(21) H(22) H(23) H(24) H(25) H(26) H(27) H(28) H(29) H(30) H(31) H(32) H(33) H(34) H(35) H(36) H(37) H(38)
H(39)

static void late(void) {
    print_cstr("registered by a handler, run next");
}

static void last(void) {
    /* (registered first: called last) */
    int i, ok = n == 40;
    for (i = 0; i < n; i++)
        ok &= order[i] == 39 - i;
    print_bool(ok);
}

static void registers(void) {
    atexit(late);
}

int main(void) {
    int i;
    void (*hs[40])(void) = {h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11, h12, h13, h14, h15, h16, h17, h18, h19,
                            h20, h21, h22, h23, h24, h25, h26, h27, h28, h29, h30, h31, h32, h33, h34, h35, h36, h37,
                            h38, h39};
    print_i64(atexit(last)); /* expect: 0 */
    for (i = 0; i < 40; i++) {
        fs[i] = hs[i];
        if (atexit(fs[i]) != 0)
            print_cstr("atexit failed");
    }
    atexit(registers);
    print_cstr("main returns"); /* expect: main returns */
    return 9;
    /* expect: registered by a handler, run next */
    /* expect: true */
}
