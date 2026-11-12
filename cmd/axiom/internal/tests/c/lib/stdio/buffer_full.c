/* Buffering (C17 7.21.3p3, p7): standard output to a file is fully
   buffered and standard error unbuffered, so the error output comes first
   when both go to one file; fflush writes the buffer; exit (here,
   returning from main) flushes what is left (7.22.4.4p4). */
/* merge-stderr */
#include <stdio.h>

int main(void) {
    printf("out 1\n");
    fprintf(stderr, "err 1\n");
    printf("out 2\n");
    fflush(stdout);
    fprintf(stderr, "err 2\n");
    printf("out 3 (at exit)\n");
    fputs("err 3\n", stderr);
    return 0;
}
/* expect: err 1 */
/* expect: out 1 */
/* expect: out 2 */
/* expect: err 2 */
/* expect: err 3 */
/* expect: out 3 (at exit) */
