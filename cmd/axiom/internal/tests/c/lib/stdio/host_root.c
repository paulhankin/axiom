/* The C library's files on a WebAssembly host that confines the program
   to a directory (the node host's --root=dir, here the test's working
   directory, which the program sees as /): files are created there, and
   no path leads out of it. */
/* wasm-only */
/* host-flags: --root=. */
#include <stdio.h>
#include "print.h"

int main(void) {
    FILE *f = fopen("/inside.txt", "w");
    print_bool(f != NULL); /* expect: true */
    fputs("x", f);
    fclose(f);
    f = fopen("inside.txt", "r");
    print_bool(f != NULL); /* expect: true */
    fclose(f);
    /* the root's parent is the root */
    f = fopen("../../../inside.txt", "r");
    print_bool(f != NULL); /* expect: true */
    fclose(f);
    print_bool(fopen("/absent.txt", "r") == NULL); /* expect: true */
    print_bool(fopen("../../../../etc/hostname", "r") == NULL); /* expect: true */
    print_i64(remove("/inside.txt")); /* expect: 0 */
    print_bool(fopen("/tmp/t.txt", "w") != NULL); /* expect: true */
    return 0;
}
