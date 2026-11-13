/* Without __STDC_WANT_LIB_EXT1__ the headers declare none of Annex K's
   names (the implementation's choice, C11 K.3.1.1p3), which stay the
   program's: here a function of its own named memcpy_s, and errno_t and
   rsize_t as its own types. */
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <wchar.h>

typedef double errno_t;
typedef char rsize_t;
#define RSIZE_MAX "mine"

static int memcpy_s(int a) {
    return a * 2;
}

int main(void) {
    errno_t e = 1.5;
    rsize_t r = 'r';
    printf("%d %g %c %s %d\n", memcpy_s(21), e, r, RSIZE_MAX, __STDC_LIB_EXT1__ == 201112L);
    return 0;
}
// expect: 42 1.5 r mine 1
