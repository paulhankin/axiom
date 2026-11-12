/* Line buffering (C17 7.21.3p3, 7.21.5.6): with setvbuf's _IOLBF,
   standard output is written at each new-line, and when input is
   requested from a line-buffered stream (standard input, made one here)
   that needs the host's characters. */
/* merge-stderr */
/* stdin: typed */
#include <stdio.h>

int main(void) {
    char buf[16];
    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stdin, NULL, _IOLBF, 0);
    printf("line 1\npart");
    fprintf(stderr, "err 1\n");
    printf(" of line 2\n");
    fprintf(stderr, "err 2\n");
    printf("prompt: ");
    if (fgets(buf, sizeof buf, stdin) != NULL)
        fprintf(stderr, "got %s", buf);
    printf("end\n");
    return 0;
}
/* expect: line 1 */
/* expect: err 1 */
/* expect: part of line 2 */
/* expect: err 2 */
/* expect: prompt: got typed */
/* expect: end */
