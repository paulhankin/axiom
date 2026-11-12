/* C23's binary prefixes where the library is C17 (its units are compiled
   as C17): in C23 mode the header maps fscanf's family, strtol's and
   wcstol's, and strtoimax's to the C23 forms, so %i and base 0 read 0b101
   (C23 7.23.6.2p12, 7.24.1.7); the same call in C17 mode reads the 0 (see
   c17_prefixes.c). */
/* flags: --c-std=c23 */
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

int main(void) {
    int i = 0, b = 0;
    char *end;
    wchar_t *wend;
    printf("%d %d %d\n", sscanf("0b101 0B11", "%i %i", &i, &b), i, b);
    printf("%ld %s\n", strtol("0b1101z", &end, 0), end);
    printf("%lld %llu\n", strtoll("-0b11", NULL, 2), strtoull("0B1111", NULL, 0));
    {
        const wchar_t *w = L"0b111";
        printf("%ld %d\n", wcstol(w, &wend, 0), (int)(wend - w));
    }
    printf("%jd %ju\n", strtoimax("0b10", NULL, 0), wcstoumax(L"0b100", NULL, 0));
    i = 0;
    printf("%d %d\n", swscanf(L"0b110", L"%i", &i), i);
    return 0;
}
/* expect: 2 5 3 */
/* expect: 13 z */
/* expect: -3 15 */
/* expect: 7 5 */
/* expect: 2 4 */
/* expect: 1 6 */
