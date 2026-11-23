/* A program of LIB_1.0's foo (built as it was, before LIB_2.0's) */
#include <stdio.h>
int foo(void);
__asm__(".symver foo, foo@LIB_1.0");
int main(void) {
    printf("%d\n", foo());
    return 0;
}
