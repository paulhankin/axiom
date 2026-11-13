/* Annex K in C23 mode: fscanf_s's family takes C23's 0b prefix for %i as
   fscanf's does (<stdio.h>, <wchar.h> map the names), and the headers
   define their __STDC_VERSION_..._H__ macros with the Annex's names. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <stdio.h>
#include <wchar.h>

int main(void) {
    int a = 0, b = 0;
    int r = sscanf_s("0b101 0x1f", "%i %i", &a, &b);
    int w = swscanf_s(L"0B11", L"%i", &a);
    printf("%d %d %d %d %d\n", r, a, b, w, __STDC_LIB_EXT1__ == 201112L);
    return 0;
}
// flags: --c-std=c23
// expect: 2 3 31 1 1
