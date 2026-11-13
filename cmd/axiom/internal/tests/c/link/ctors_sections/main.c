/* Constructors and destructors of an object of the system's C compiler
   (its .init_array and .fini_array, with priorities) and of this
   compiler's (--emit=obj: its own arrays), before and after main, in the
   order of their priorities; and a section whose __start_ and __stop_
   symbols the object refers to. */
/* ours-flags: --c-std=gnu17 */
/* cc-variants: -O2 | -O0 -fPIC */
#include <stdio.h>

int sec_sum(void);

int main(void) {
    /* expect: sys 101 */
    /* expect: ours 150 */
    /* expect: sys 200 */
    /* expect: sys */
    /* expect: ours */
    printf("main %d\n", sec_sum()); /* expect: main 66 */
    /* expect: ours dtor */
    /* expect: sys dtor */
    return 0;
}
