/* A C89 program with the hosted library: <stdio.h> declares what C89 has
   (gets among them, removed by C11), and none of C99's names, which the
   program may then use for itself. */
/* flags: --c-std=c89 */
#include <stdio.h>

static int snprintf(int x) { return x + 1; }
static int vscanf;

int main(void) {
    char buf[8];
    vscanf = snprintf(1);
    sprintf(buf, "%d%c", vscanf, '!');
    puts(buf);
    printf("%s %ld %lu\n", gets == 0 ? "no gets" : "gets", -1L, 1UL);
    return 0;
}
/* expect: 2! */
/* expect: gets -1 1 */
