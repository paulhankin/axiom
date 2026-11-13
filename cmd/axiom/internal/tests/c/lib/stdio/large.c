/* One call of more than 2^31 bytes: fwrite of 3 GB (to /dev/null, from
   memory never touched) writes them all; and a formatted output of more
   than INT_MAX characters fails with EOVERFLOW (C17 7.21.6.1p14,
   7.21.6.5p3), whether to a stream or counted by snprintf. */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

int main(void) {
    size_t n = (size_t)3 << 30;
    char *p = malloc(n);
    FILE *f = fopen("/dev/null", "wb");
    size_t w;
    int r;
    if (p == NULL || f == NULL)
        return 1;
    fputc('x', f);
    w = fwrite(p, 1, n, f);
    printf("fwrite %d\n", w == n);
    fclose(f);
    errno = 0;
    r = snprintf(NULL, 0, "%*d%*d", INT_MAX, 1, 2, 3);
    printf("snprintf %d %d\n", r, errno == EOVERFLOW);
    errno = 0;
    r = snprintf(NULL, 0, "%*d", INT_MAX, 1);
    printf("snprintf INT_MAX %d %d\n", r == INT_MAX, errno == 0);
    f = fopen("/dev/null", "w");
    errno = 0;
    r = fprintf(f, "%*d%*d", INT_MAX - 5, 1, 10, 3);
    printf("fprintf %d %d\n", r, errno == EOVERFLOW);
    fclose(f);
    free(p);
    return 0;
}
// native-only
// posix-only
// expect: fwrite 1
// expect: snprintf -1 1
// expect: snprintf INT_MAX 1 1
// expect: fprintf -1 1
