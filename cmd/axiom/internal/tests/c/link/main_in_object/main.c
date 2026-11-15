/* main in an object of this compiler, the program's other code a source
   (axiom build util.c main.o): no C3063, the source's functions are the
   object's main's, and main gets the program's start-up. */
/* main-object: ours */
/* flags: --c-std=gnu17 */
/* flag-variants: -O0 | -O2 --c-lib=system */
#include <stdio.h>

int util_value(void);
int util_calls(void);

int main(int argc, char **argv) {
    (void)argv;
    printf("%d %d\n", util_value(), argc);
    printf("%d", util_calls());
    return util_value() - 40;
}
/* expect: 42 1 */
/* expect: 1 */
/* expect-status: 2 */
