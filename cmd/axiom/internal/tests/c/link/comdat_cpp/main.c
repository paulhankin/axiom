/* Objects of the system's C++ compiler with an inline function and its
   static variable in COMDAT groups: the link keeps one copy of each. */
/* needs: c++ */
/* cc-variants: -O0 | -O2 */
#include <stdio.h>

int cpp_one(int);
int cpp_two(int);

int main(void) {
    int a = cpp_one(1);
    int b = cpp_two(1);
    int c = cpp_one(1);
    printf("%d %d %d\n", a, b, c); /* expect: 3 4 5 */
    return 0;
}
