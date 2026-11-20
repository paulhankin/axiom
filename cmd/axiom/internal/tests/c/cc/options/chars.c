#include <limits.h>
#include <stdio.h>
/* plain char's signedness (-funsigned-char) */
int add(int a, int b) { return a + b; }
int main(void) {
    char c = (char)200;
    printf("%d %d\n", c > 0, CHAR_MIN);
    return 0;
}
