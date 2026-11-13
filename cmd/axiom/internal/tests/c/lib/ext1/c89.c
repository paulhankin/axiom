/* Annex K's interfaces in C89 mode, asked for: the headers declare them
   in every mode (with C89's and C99's keywords), but __STDC_LIB_EXT1__
   is a macro of C11 and later (6.10.8.3) and not defined. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>
#include <stdio.h>

int main(void) {
    char a[8];
    errno_t e = strcpy_s(a, sizeof a, "c89");
#ifdef __STDC_LIB_EXT1__
    printf("defined\n");
#endif
    printf("%d %s %d\n", e, a, (int)strnlen_s(a, 2));
    return 0;
}
/* flags: --c-std=c89 */
/* expect: 0 c89 2 */
