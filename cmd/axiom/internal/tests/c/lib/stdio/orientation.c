/* Orientation (C17 7.21.2p4-p5, 7.29.3.5): fwide queries and sets it once;
   every byte input/output function fails on a wide-oriented stream, and
   every wide one on a byte-oriented stream, reading and writing nothing;
   freopen makes a stream unoriented again. The results are printed with
   wprintf on a wide standard output. */
#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>
#include <string.h>

static int vf(FILE *f, const char *fmt, ...) {
    va_list ap;
    int r;
    va_start(ap, fmt);
    r = vfprintf(f, fmt, ap);
    va_end(ap);
    return r;
}

static int vs(FILE *f, const char *fmt, ...) {
    va_list ap;
    int r;
    va_start(ap, fmt);
    r = vfscanf(f, fmt, ap);
    va_end(ap);
    return r;
}

static int wvf(FILE *f, const wchar_t *fmt, ...) {
    va_list ap;
    int r;
    va_start(ap, fmt);
    r = vfwprintf(f, fmt, ap);
    va_end(ap);
    return r;
}

static int wvs(FILE *f, const wchar_t *fmt, ...) {
    va_list ap;
    int r;
    va_start(ap, fmt);
    r = vfwscanf(f, fmt, ap);
    va_end(ap);
    return r;
}

int main(void) {
    FILE *f = fopen("o.txt", "w+");
    char b[8];
    wchar_t w[8];
    int x = 0;
    long pos;
    /* a wide stream: the byte functions fail */
    int q0 = fwide(f, 0), q1 = fwide(f, 1), q2 = fwide(f, -1), q3 = fwide(f, 0);
    fputws(L"12 ab\n", f);
    rewind(f);
    pos = ftell(f);
    {
        int r[15];
        r[0] = fgetc(f) == EOF;
        r[1] = getc(f) == EOF;
        r[2] = fgets(b, sizeof b, f) == NULL;
        r[3] = fputc('x', f) == EOF;
        r[4] = putc('x', f) == EOF;
        r[5] = fputs("x", f) == EOF;
        r[6] = ungetc('x', f) == EOF;
        r[7] = fread(b, 1, 2, f) == 0;
        r[8] = fwrite("x", 1, 1, f) == 0;
        r[9] = fprintf(f, "x") < 0;
        r[10] = vf(f, "x") < 0;
        r[11] = fscanf(f, "%d", &x) == EOF;
        r[12] = vs(f, "%d", &x) == EOF;
        r[13] = ftell(f) == pos;
        r[14] = fwscanf(f, L"%d", &x) == 1 && x == 12;
        fclose(f);
        /* a byte stream: the wide functions fail */
        f = fopen("o.txt", "r+");
        fputs("34 cd\n", f);
        rewind(f);
        {
            int s[14];
            s[0] = fwide(f, 0) < 0;
            s[1] = fgetwc(f) == WEOF;
            s[2] = getwc(f) == WEOF;
            s[3] = fgetws(w, 8, f) == NULL;
            s[4] = fputwc(L'x', f) == WEOF;
            s[5] = putwc(L'x', f) == WEOF;
            s[6] = fputws(L"x", f) < 0;
            s[7] = ungetwc(L'x', f) == WEOF;
            s[8] = fwprintf(f, L"x") < 0;
            s[9] = wvf(f, L"x") < 0;
            s[10] = fwscanf(f, L"%d", &x) == EOF;
            s[11] = wvs(f, L"%d", &x) == EOF;
            s[12] = fscanf(f, "%d", &x) == 1 && x == 34;
            s[13] = fwide(f, 1) < 0;
            /* freopen: no orientation again */
            f = freopen("o.txt", "r", f);
            x = fwide(f, 0);
            /* stdout made wide: putchar, puts and printf fail on it */
            fwide(stdout, 1);
            {
                int t0 = putchar('x') == EOF, t1 = puts("x") == EOF, t2 = printf("x") < 0;
                int i;
                wprintf(L"fwide %d %d %d %d\n", q0, q1 > 0, q2 > 0, q3 > 0);
                wprintf(L"byte on wide:");
                for (i = 0; i < 15; i++)
                    wprintf(L" %d", r[i]);
                wprintf(L"\nwide on byte:");
                for (i = 0; i < 14; i++)
                    wprintf(L" %d", s[i]);
                wprintf(L"\nfreopen %d\nstdout %d %d %d\n", x, t0, t1, t2);
            }
        }
    }
    fclose(f);
    remove("o.txt");
    return 0;
}
// expect: fwide 0 1 1 1
// expect: byte on wide: 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
// expect: wide on byte: 1 1 1 1 1 1 1 1 1 1 1 1 1 1
// expect: freopen 0
// expect: stdout 1 1 1
