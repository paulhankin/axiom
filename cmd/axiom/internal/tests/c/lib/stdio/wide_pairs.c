/* %lc and %ls with characters beyond U+FFFF, which a 16-bit wchar_t
   (Windows) holds as surrogate pairs: fscanf's %ls stores both halves of
   one multibyte character (mbrtowc's (size_t)-3 step), fprintf's %ls
   writes the pair as one UTF-8 character (wcrtomb keeps the first half),
   and %lc of each half alone writes nothing for the first and EILSEQ's
   failure for a second that has no first. With a 32-bit wchar_t the
   character is one element. */
#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>

int main(void) {
    wchar_t w[8];
    char out[32];
    int r, n;
    size_t elems = WCHAR_MAX > 0xFFFF ? 1 : 2;
    setlocale(LC_ALL, "C.UTF-8");
    r = sscanf("\xF0\x9F\x98\x80z", "%4ls%n", w, &n);
    printf("sscanf %d %d %d\n", r, n, wcslen(w) == elems);
    r = snprintf(out, sizeof out, "[%ls]", w);
    printf("snprintf %d %d\n", r, strcmp(out, "[\xF0\x9F\x98\x80]") == 0);
    r = sscanf("a\xF0\x9F\x98\x80" "b", "%ls", w);
    printf("wide string %d %d\n", r, wcslen(w) == elems + 2);
    r = snprintf(out, sizeof out, "%ls", w);
    printf("back %d %d\n", r, strcmp(out, "a\xF0\x9F\x98\x80" "b") == 0);
    r = swprintf(w, 8, L"%s", "\xF0\x9F\x98\x80");
    printf("swprintf %d\n", r == (int)elems);
    {
        /* a wide stream: fgetwc gives both halves, then the next byte's */
        FILE *f = fopen("p.txt", "wb");
        wint_t a, b, c;
        fputs("\xF0\x9F\x98\x80z", f);
        fclose(f);
        f = fopen("p.txt", "rb");
        a = fgetwc(f);
        b = fgetwc(f);
        c = fgetwc(f);
        printf("fgetwc %d\n", elems == 1 ? a == 0x1F600 && b == L'z' && c == WEOF
                                          : a == 0xD83D && b == 0xDE00 && c == L'z');
        fclose(f);
        remove("p.txt");
    }
    if (elems == 2) {
        r = snprintf(out, sizeof out, "%lc", (wint_t)0xD83D);
        n = snprintf(out, sizeof out, "%lc", (wint_t)0xDE00);
        printf("halves %d %d\n", r, n);
    } else {
        printf("halves 0 -1\n");
    }
    return 0;
}
// expect: sscanf 1 4 1
// expect: snprintf 6 1
// expect: wide string 1 1
// expect: back 6 1
// expect: swprintf 1
// expect: fgetwc 1
// expect: halves 0 -1
