/* Positions past 4 GB (sparse files, where the file system allows): fseek
   by steps that long holds, fgetpos and fsetpos, and ftell, which fails
   with EOVERFLOW when long cannot hold the position (a 32-bit long:
   wasm32 here, Windows), else gives it. */
#include <stdio.h>
#include <errno.h>
#include <limits.h>

int main(void) {
    FILE *f = fopen("big.bin", "w+b");
    fpos_t at, start;
    long t;
    int i, ok = 1;
    fgetpos(f, &start);
    for (i = 0; i < 5; i++)
        ok &= fseek(f, 1000000000L, SEEK_CUR) == 0;
    fputc('x', f);
    fgetpos(f, &at);
    errno = 0;
    t = ftell(f);
    if (LONG_MAX > 2147483647L)
        printf("seek %d, ftell %d\n", ok, t == 5000000001L);
    else
        printf("seek %d, ftell %d\n", ok, t == -1L && errno == EOVERFLOW);
    fsetpos(f, &start);
    fsetpos(f, &at);
    fseek(f, -1, SEEK_CUR);
    printf("read back %c\n", fgetc(f));
    printf("then end of file %d\n", fgetc(f) == EOF);
    fclose(f);
    remove("big.bin");
    return 0;
}
/* expect: seek 1, ftell 1 */
/* expect: read back x */
/* expect: then end of file 1 */
