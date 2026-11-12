/* Wide-character input/output (C17 7.29.2, 7.29.3) in the "C" locale,
   and the orientation of streams (7.21.2p4): the first byte or wide
   operation decides it, fwide queries and sets it, the other kind of
   operation then fails, freopen clears it. swprintf's n, swscanf. */
#include <stdio.h>
#include <wchar.h>

int main(void) {
    FILE *f = tmpfile();
    wchar_t ws[32];
    char buf[32];
    int r, a = 0;
    wint_t c;
    printf("none: %d\n", fwide(f, 0));
    fputwc(L'w', f);
    printf("wide: %d %d\n", fwide(f, 0) > 0, fwide(f, -1) > 0);
    r = fputc('x', f) == EOF;
    r += fputs("x", f) == EOF;
    r += fprintf(f, "x") < 0;
    printf("byte functions fail: %d\n", r);
    fputws(L"ide\n", f);
    r = fwprintf(f, L"%d %ls %s %5.2f|%-3lc|\n", 42, L"wide", "narrow", 2.5, (wint_t)L'c');
    printf("fwprintf: %d\n", r);
    rewind(f);
    printf("[%ls]", fgetws(ws, 32, f));
    c = fgetwc(f);
    printf("[%lc", (wint_t)c);
    printf("%ls]\n", fgetws(ws, 32, f));
    r = fgetwc(f) == WEOF;
    printf("eof: %d %d\n", r, feof(f) != 0);
    printf("ungetwc: %d", ungetwc(L'Q', f) == L'Q');
    c = getwc(f);
    printf(" %lc %d\n", (wint_t)c, getwc(f) == WEOF);
    rewind(f);
    r = fwscanf(f, L"%ls %d %ls", ws, &a, ws);
    printf("fwscanf: %d %d %ls\n", r, a, ws);
    fclose(f);
    /* a byte-oriented stream refuses wide operations */
    f = tmpfile();
    fputs("bytes", f);
    r = fwide(f, 1) < 0;
    r += fputwc(L'x', f) == WEOF;
    r += fwprintf(f, L"x") < 0;
    printf("byte: %d\n", r);
    f = freopen(NULL, "w+", f);
    printf("freopen: %d\n", f != NULL && fwide(f, 0) == 0);
    fclose(f);
    /* swprintf: at most n wide characters with the null; negative if more */
    r = swprintf(ws, 32, L"%d-%ls-%s-%c", 7, L"ab", "cd", 'e');
    printf("swprintf: %d %ls\n", r, ws);
    r = swprintf(ws, 4, L"%d", 12345);
    printf("too long: %d %ls\n", r < 0, ws);
    r = swscanf(L" 12 three 4.5", L"%d %5ls %s", &a, ws, buf);
    printf("swscanf: %d %d %ls %s\n", r, a, ws, buf);
    r = swscanf(L"abc]de", L"%l[a-c]%lc", ws, ws + 10);
    printf("scanset: %d %ls %lc\n", r, ws, (wint_t)ws[10]);
    r = sscanf("narrow", "%3ls", ws);
    printf("%%ls in sscanf: %d %ls\n", r, ws);
    r = wprintf(L"x") < 0;
    printf("wprintf on the byte-oriented stdout fails: %d\n", r);
    return 0;
}
/* expect: none: 0 */
/* expect: wide: 1 1 */
/* expect: byte functions fail: 3 */
/* expect: fwprintf: 26 */
/* expect: [wide */
/* expect: ][42 wide narrow  2.50|c  | */
/* expect: ] */
/* expect: eof: 1 1 */
/* expect: ungetwc: 1 Q 1 */
/* expect: fwscanf: 3 42 wide */
/* expect: byte: 3 */
/* expect: freopen: 1 */
/* expect: swprintf: 9 7-ab-cd-e */
/* expect: too long: 1 123 */
/* expect: swscanf: 3 12 three 4.5 */
/* expect: scanset: 2 abc ] */
/* expect: %ls in sscanf: 1 nar */
/* expect: wprintf on the byte-oriented stdout fails: 1 */
