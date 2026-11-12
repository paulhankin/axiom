/* Wide-character input/output in a UTF-8 locale ("C.UTF-8", the text
   part's): fputws and fwprintf write UTF-8, fgetwc reads it back, ungetwc
   takes back a character of several bytes, printf's %ls and %lc convert
   (the width and precision count bytes),
   sscanf's %ls and swscanf's %s convert the other way, and an invalid
   sequence is an encoding error (EILSEQ). */
#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <wchar.h>

int main(void) {
    FILE *f;
    wchar_t ws[16];
    char buf[32];
    wint_t c, d;
    int n;
    printf("%s\n", setlocale(LC_ALL, "C.UTF-8") != NULL ? "utf8" : "none");
    n = printf("[%ls][%.3ls][%lc][%5ls]\n", L"hé€", L"éé", (wint_t)0x20ac, L"€");
    printf("%d\n", n);
    f = tmpfile();
    fputws(L"aé€üz\n", f);
    fwprintf(f, L"%s|%ls|%c\n", "\xc3\xa9t\xc3\xa9", L"π", 'x');
    rewind(f);
    while ((c = fgetwc(f)) != WEOF)
        printf("%lx ", (unsigned long)c);
    printf("\n");
    rewind(f);
    fgetwc(f);
    c = fgetwc(f);
    ungetwc(c, f);
    c = fgetwc(f);
    d = fgetwc(f);
    printf("%lx %lx\n", (unsigned long)c, (unsigned long)d);
    fclose(f);
    n = sscanf("\xc3\xa9t\xc3\xa9 x", "%ls", ws);
    printf("%d %d %lx\n", n, (int)wcslen(ws), (unsigned long)ws[0]);
    n = swscanf(L"été y", L"%s", buf);
    printf("%d %s\n", n, buf);
    f = tmpfile();
    fwide(f, 1);
    fputwc(L'a', f);
    fclose(f);
    f = tmpfile();
    fputs("\xc3\x28", f);
    fclose(f);
    f = tmpfile();
    fprintf(f, "%s", "\xc3\x28");
    rewind(f);
    f = freopen(NULL, "r", f);
    errno = 0;
    c = fgetwc(f);
    printf("invalid: %d %d\n", c == WEOF, errno == EILSEQ);
    fclose(f);
    return 0;
}
/* expect: utf8 */
/* expect: [hé€][é][€][  €] */
/* expect: 25 */
/* expect: 61 e9 20ac fc 7a a e9 74 e9 7c 3c0 7c 78 a  */
/* expect: e9 20ac */
/* expect: 1 3 e9 */
/* expect: 1 été */
/* expect: invalid: 1 1 */
