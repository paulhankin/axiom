/* No buffering (C17 7.21.3p3, 7.21.5.6): with setvbuf's _IONBF each
   output is written at once (a formatted one in one piece), so the order
   of the two streams in one file is the program's; a buffer of the
   program's given to setvbuf; setbuf. */
/* merge-stderr */
#include <stdio.h>

static char mine[64];

int main(void) {
    FILE *f;
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("a%d", 1);
    fprintf(stderr, "b");
    putchar('c');
    fputs("d", stderr);
    fwrite("e\n", 1, 2, stdout);
    f = tmpfile();
    printf("setvbuf: %d\n", setvbuf(f, mine, _IOFBF, sizeof mine));
    fputs("in the program's buffer", f);
    printf("[%s]\n", mine);
    fclose(f);
    f = tmpfile();
    setbuf(f, NULL);
    fputs("unbuffered", f);
    rewind(f);
    printf("%c\n", getc(f));
    fclose(f);
    printf("bad mode: %d\n", setvbuf(stdout, NULL, 3, 0) != 0);
    return 0;
}
/* expect: a1bcde */
/* expect: setvbuf: 0 */
/* expect: [in the program's buffer] */
/* expect: u */
/* expect: bad mode: 1 */
