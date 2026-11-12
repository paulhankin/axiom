/* C17 mode: 0b is no prefix (C17 7.22.1.4): strtol with base 0 and %i read
   the 0 and stop at the b (see c23_prefixes.c, C23's). */
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int i = 7;
    char *end;
    printf("%d %d\n", sscanf("0b101", "%i", &i), i);
    printf("%ld %s\n", strtol("0b1101", &end, 0), end);
    return 0;
}
/* expect: 1 0 */
/* expect: 0 b1101 */
