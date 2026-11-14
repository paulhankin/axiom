/* flags: --c-std=c23 */
/* host-flags: --files=memory */
/* Variadic logging and formatting: a logger whose va_list is passed down
   several levels and copied with va_copy (formatted twice: to measure, then
   to write), every printf conversion with its flags, widths and precisions
   (from the format and from '*' arguments), %n, %a, wide strings, C23's %b
   and length modifiers and the decimal floating values, and scanf round
   trips. The output was compared with the system's compiler and library
   (but for the decimal types, which it has not) at -O0 and -O2; what
   depends on the width of long or long double is not printed, and of the
   choices the standard leaves, %La of 1.0 is 0x1p+0 here (0x8p-3 is
   another library's), -nan is not printed. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <wchar.h>

enum level { DEBUG, INFO, WARN, ERROR };
static const char *const lname[] = { "debug", "info", "warn", "error" };
static char logbuf[4096];
static size_t loglen;
static int lines;

static void sink(const char *s, size_t n) {
    if (loglen + n < sizeof logbuf) { memcpy(logbuf + loglen, s, n); loglen += n; logbuf[loglen] = 0; }
}

static int vformat_into(char **out, const char *fmt, va_list ap) {
    va_list ap2;
    va_copy(ap2, ap);
    int n = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);
    if (n < 0) return n;
    *out = malloc((size_t)n + 1);
    int m = vsnprintf(*out, (size_t)n + 1, fmt, ap);
    return m == n ? n : -1;
}

static void vlog_at(enum level lv, const char *file, int line, const char *fmt, va_list ap) {
    char *msg;
    int n = vformat_into(&msg, fmt, ap);
    if (n < 0) { sink("format error\n", 13); return; }
    char head[64];
    int h = snprintf(head, sizeof head, "[%-5s %s:%d] ", lname[lv], file, line);
    sink(head, (size_t)h);
    sink(msg, (size_t)n);
    sink("\n", 1);
    lines++;
    free(msg);
}

static void vlog(enum level lv, const char *file, int line, const char *fmt, va_list ap) {
    va_list again;
    va_copy(again, ap);
    if (lv >= WARN) {
        /* warnings go to stdout too, from the copy */
        printf("! ");
        vprintf(fmt, again);
        putchar('\n');
    }
    va_end(again);
    vlog_at(lv, file, line, fmt, ap);
}

static void logf_(enum level lv, const char *file, int line, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vlog(lv, file, line, fmt, ap);
    va_end(ap);
}
#define LOG(lv, ...) logf_(lv, "p9", __LINE__ - __LINE__ + 1, __VA_ARGS__)

/* a variadic function walking a typed list */
static double sum_typed(const char *types, ...) {
    va_list ap;
    va_start(ap, types);
    double s = 0;
    for (const char *t = types; *t; t++) {
        switch (*t) {
        case 'i': s += va_arg(ap, int); break;
        case 'l': s += (double)va_arg(ap, long long); break;
        case 'd': s += va_arg(ap, double); break;
        case 'L': s += (double)va_arg(ap, long double); break;
        case 'p': s += *va_arg(ap, int *); break;
        case 's': { struct big { int a[5]; } b = va_arg(ap, struct big); s += b.a[0] + b.a[4]; break; }
        case 'c': s += (char)va_arg(ap, int); break;
        }
    }
    va_end(ap);
    return s;
}
struct big { int a[5]; };

static int count_args(int n, ...) {
    va_list a, b;
    va_start(a, n);
    va_copy(b, a);
    int s = 0;
    for (int i = 0; i < n; i++) s += va_arg(a, int);
    for (int i = 0; i < n; i++) s -= va_arg(b, int) * 2;
    va_end(a);
    va_end(b);
    return s;
}

static void conversions(void) {
    int n1 = 0, n2 = 0;
    printf("[%d|%i|%5d|%-5d|%05d|%+d|% d|%+05d|%.3d|%8.3d|%-8.3d|%.0d|%x|%X|%#x|%#o|%o|%u]\n", 42, -42, 42, 42, 42, 42, 42, -42, 7, 7, 7, 0, 255, 255, 255, 8, 8, 3000000000u);
    printf("[%hhd|%hhu|%hd|%hu|%ld|%lu|%lld|%llu|%jd|%ju|%zd|%zu|%td|%tx]\n", (signed char)-129 + 1, (unsigned char)300, (short)70000, (unsigned short)70000,
           -1L, 1UL, LLONG_MIN, ULLONG_MAX, (intmax_t)INTMAX_MAX, (uintmax_t)42, (ptrdiff_t)-5, sizeof(int), (ptrdiff_t)-1, (ptrdiff_t)255);
    printf("[%c|%3c|%-3c|%s|%10s|%-10s|%.2s|%10.2s|%%|%n]\n", 'x', 'y', 'z', "str", "right", "left", "precise", "pr", &n1);
    printf("n1=%d\n", n1);
    printf("[%f|%.0f|%#.0f|%10.3f|%-10.3f|%+.2f|% .2f|%010.2f|%F]\n", 3.14159, 2.5, 2.5, -1.0005, 1.0005, 1.0, 1.0, -3.5, INFINITY);
    printf("[%e|%.0e|%#.0e|%E|%.10e|%12.3e|%-12.3E]\n", 12345.678, 5e10, 5e10, 1e-300, 1.0 / 3, -0.000123, 6.02e23);
    printf("[%g|%g|%g|%g|%#g|%.3g|%.10g|%G|%g|%g]\n", 100000.0, 1000000.0, 0.0001, 0.00001, 1.0, 3.14159, 1.0 / 7, 1e-10, -0.0, NAN);
    printf("[%a|%A|%.3a|%.0a|%a|%a|%a|%#a|%20a|%-20a|]\n", 1.0, 255.5, 1.0 / 3, 1.5, DBL_MIN, DBL_TRUE_MIN, -0.0, 2.0, 1e-5, 3.0);
    printf("[%La|%Lf|%.15Lg]\n", 1.0L, 0.1L, 1.0L / 3);
    printf("[%*d|%-*d|%.*f|%*.*e|%*s|%-*s|]\n", 6, 1, 6, 2, 3, 3.14159, 12, 2, 31415.9, -6, "neg", 4, "ab");
    printf("[%s]\n", (void *)0 == NULL ? "null pointer prints implementation-defined" : "?");
    printf("%s%n%s%hhn|%d %d\n", "abc", &n2, "de", (signed char *)&n1, n2, (signed char)n1);
    printf("[%lc|%ls|%5ls|%-5.2ls|]\n", (wint_t)L'w', L"wide", L"ab", L"wxyz");
    printf("[%.0f|%.0f|%.0f|%.0f|%.1f|%.1f|%.2f]\n", 0.5, 1.5, 2.5, -0.5, 0.05, 0.25, 1.005);
    printf("[%.17g|%.17g|%.0f]\n", 0.1, 1e23, 1e23);
    printf("[%.40f]\n", 1.0 / 3);
    printf("[%.3s|%.*s]\n", "abcdef", 2, "xyz");
    printf("[%x|%llx|%jx]\n", UINT_MAX, ULLONG_MAX, UINTMAX_MAX);
    printf("[%" PRId8 "|%" PRIu16 "|%" PRIx32 "|%" PRIX64 "|%" PRIdLEAST8 "|%" PRIuFAST16 "|%" PRIdMAX "|%" PRIxPTR "]\n", INT8_MIN, UINT16_MAX, UINT32_MAX,
           UINT64_MAX, (int_least8_t)-1, (uint_fast16_t)65535, INTMAX_MIN, (uintptr_t)0xbeef);
    printf("[%b|%#b|%B|%#B|%08b|%.4b|%w8d|%w16x|%w32d|%w64u|%wf8d|%wf32x]\n", 10u, 10u, 5u, 5u, 3u, 1u, (int8_t)-8, (uint16_t)0xFFFF, INT32_MIN, UINT64_MAX,
           (int_fast8_t)7, (uint_fast32_t)0xABCDu);
    int bc;
    printf("scanf %%b: %d", sscanf("1011", "%b", &bc));
    printf(" -> %d\n", bc);
#ifdef __STDC_IEC_60559_DFP__
    _Decimal64 d64 = 1.10DD;
    _Decimal32 d32 = 0.1DF;
    _Decimal128 d128 = 3.14159265358979323846264338327950DL;
    printf("[%Da|%Df|%Hg|%De|%.10Df|%Dg|%DDe|%.30DDf]\n", d64, d64 * 3, d32, d64 / 3, d64 / 3, 1e300DD * 10, d128, d128);
#else
    puts("no decimal floating point");
#endif
}

static void scanning(void) {
    char text[512];
    double vals[] = { 0.0, -0.0, 1.5, -2.25e-10, 123456789.0, 1e300, 5e-324, 0x1.fffffffffffffp+1023 };
    int pos = 0;
    for (size_t i = 0; i < sizeof vals / sizeof vals[0]; i++) pos += snprintf(text + pos, sizeof text - (size_t)pos, "%a %.17e ", vals[i], vals[i]);
    const char *p = text;
    int ok = 0;
    for (size_t i = 0; i < sizeof vals / sizeof vals[0]; i++) {
        double a, b; int used;
        if (sscanf(p, "%la %le%n", &a, &b, &used) == 2 && a == vals[i] && b == vals[i] && signbit(a) == signbit(vals[i])) ok++;
        p += used;
    }
    printf("float round trips %d/%zu\n", ok, sizeof vals / sizeof vals[0]);
    unsigned long long u; long long s; unsigned o; int x; char c[4]; char word[16]; int n;
    int k = sscanf("18446744073709551615 -9223372036854775808 0777 0x1f abc rest", "%llu %lld %o %x %3c %15s%n", &u, &s, &o, &x, c, word, &n);
    printf("scan %d: %llu %lld %o %x '%.3s' '%s' n=%d\n", k, u, s, o, x, c, word, n);
    k = sscanf("  42  ,  17", "%d ,%d", &x, &n);
    printf("whitespace %d %d %d\n", k, x, n);
    k = sscanf("abc123", "%[a-z]%d", word, &x);
    printf("scanset %d '%s' %d\n", k, word, x);
    k = sscanf("x", "%d", &x);
    printf("mismatch %d\n", k);
    k = sscanf("12abc", "%d%n%*s%n", &x, &n, &o);
    printf("suppressed %d %d %u\n", k, n, o);
    float f; long double ld; short hs; signed char hc; size_t z; ptrdiff_t t; intmax_t j;
    k = sscanf("1e-2 0x1p-3 -32768 -128 99 -7 123", "%f %Lf %hd %hhd %zu %td %jd", &f, &ld, &hs, &hc, &z, &t, &j);
    printf("modifiers %d: %g %Lg %hd %hhd %zu %td %jd\n", k, f, ld, hs, hc, z, t, j);
    wchar_t ws[16];
    k = sscanf("wide text", "%ls", ws);
    printf("wide scan %d '%ls'\n", k, ws);
    k = sscanf("nan inf -infinity", "%lf %lf %lf", &vals[0], &vals[1], &vals[2]);
    printf("special %d %d %d %d\n", k, isnan(vals[0]), isinf(vals[1]) && vals[1] > 0, isinf(vals[2]) && vals[2] < 0);
    /* fscanf from a stream */
    FILE *fp = tmpfile();
    fprintf(fp, "%d %s %g\n%5.2f|%c\n", 7, "seven", 7.5, 3.14159, 'q');
    rewind(fp);
    char w2[16]; double g, h; char q;
    k = fscanf(fp, "%d %15s %lf %lf|%c", &x, w2, &g, &h, &q);
    printf("fscanf %d: %d %s %g %g %c, eof next %d\n", k, x, w2, g, h, q, fscanf(fp, "%d", &x));
    fclose(fp);
}

static void edge(void) {
    char small[5];
    int n = snprintf(small, sizeof small, "%d", 123456789);
    printf("truncated %d '%s'\n", n, small);
    n = snprintf(small, 0, "%s", "anything");
    printf("size 0 -> %d\n", n);
    char big[600];
    n = snprintf(big, sizeof big, "%500d|", 1);
    printf("wide field %d last '%s'\n", n, big + 498);
    n = snprintf(big, sizeof big, "%.500f", 1.0);
    printf("long precision %d ends %c%c\n", n, big[n - 2], big[n - 1]);
    n = snprintf(big, sizeof big, "%f", DBL_MAX);
    printf("DBL_MAX %%f has %d characters, starts %.12s\n", n, big);
    n = snprintf(big, sizeof big, "%.0e|%.30e", 9.5, 9.5);
    printf("%s\n", big);
    wchar_t wb[64];
    n = swprintf(wb, 64, L"%d %ls %s %5.1f %lc", -1, L"wide", "narrow", 2.25, (wint_t)L'z');
    printf("swprintf %d '%ls'\n", n, wb);
    n = swprintf(wb, 3, L"%s", "abcdef");
    printf("swprintf overflow %d\n", n < 0);
}

int main(void) {
    LOG(INFO, "starting %s v%d.%d", "logger", 1, 2);
    LOG(DEBUG, "values %5.2f %e %g %a", 3.14159, 2.71828, 1e-5, 0.5);
    LOG(WARN, "low memory: %zu bytes left (%d%%)", (size_t)1024, 5);
    LOG(ERROR, "%s: code %#x at %s", "failure", 0xdead, (void *)0 == NULL ? "here" : "");
    for (int i = 0; i < 3; i++) LOG(INFO, "loop %d of %d: %-6s|%6s|", i + 1, 3, "ab", "cd");
    fputs(logbuf, stdout);
    printf("%d lines, %zu bytes\n", lines, loglen);
    int seven = 7;
    struct big b = { { 1, 2, 3, 4, 5 } };
    printf("sum_typed %.4f\n", sum_typed("ildLpsc", 1, 2LL, 3.5, 4.25L, &seven, b, 'A'));
    printf("count_args %d\n", count_args(5, 1, 2, 3, 4, 5));
    conversions();
    scanning();
    edge();
    return 0;
}
// expect: ! low memory: 1024 bytes left (5%)
// expect: ! failure: code 0xdead at here
// expect: [info  p9:1] starting logger v1.2
// expect: [debug p9:1] values  3.14 2.718280e+00 1e-05 0x1p-1
// expect: [warn  p9:1] low memory: 1024 bytes left (5%)
// expect: [error p9:1] failure: code 0xdead at here
// expect: [info  p9:1] loop 1 of 3: ab    |    cd|
// expect: [info  p9:1] loop 2 of 3: ab    |    cd|
// expect: [info  p9:1] loop 3 of 3: ab    |    cd|
// expect: 7 lines, 297 bytes
// expect: sum_typed 88.7500
// expect: count_args -15
// expect: [42|-42|   42|42   |00042|+42| 42|-0042|007|     007|007     ||ff|FF|0xff|010|10|3000000000]
// expect: [-128|44|4464|4464|-1|1|-9223372036854775808|18446744073709551615|9223372036854775807|42|-5|4|-1|ff]
// expect: [x|  y|z  |str|     right|left      |pr|        pr|%|]
// expect: n1=53
// expect: [3.141590|2|2.|    -1.000|1.000     |+1.00| 1.00|-000003.50|INF]
// expect: [1.234568e+04|5e+10|5.e+10|1.000000E-300|3.3333333333e-01|  -1.230e-04|6.020E+23   ]
// expect: [100000|1e+06|0.0001|1e-05|1.00000|3.14|0.1428571429|1E-10|-0|nan]
// expect: [0x1p+0|0X1.FFP+7|0x1.555p-2|0x2p+0|0x1p-1022|0x0.0000000000001p-1022|-0x0p+0|0x1.p+1|0x1.4f8b588e368f1p-17|0x1.8p+1            |]
// expect: [0x1p+0|0.100000|0.333333333333333]
// expect: [     1|2     |3.142|    3.14e+04|neg   |ab  |]
// expect: [null pointer prints implementation-defined]
// expect: abcde|0 53
// expect: [w|wide|   ab|wx   |]
// expect: [0|2|2|-0|0.1|0.2|1.00]
// expect: [0.10000000000000001|9.9999999999999992e+22|99999999999999991611392]
// expect: [0.3333333333333333148296162562473909929395]
// expect: [abc|xy]
// expect: [ffffffff|ffffffffffffffff|ffffffffffffffff]
// expect: [-128|65535|ffffffff|FFFFFFFFFFFFFFFF|-1|65535|-9223372036854775808|beef]
// expect: [1010|0b1010|101|0B101|00000011|0001|-8|ffff|-2147483648|18446744073709551615|7|abcd]
// expect: scanf %b: 1 -> 11
// expect: [1.10|3.300000|0.1|3.666667e-01|0.3666666667|1e+301|3.141593e+00|3.141592653589793238462643383280]
// expect: float round trips 8/8
// expect: scan 6: 18446744073709551615 -9223372036854775808 777 1f 'abc' 'rest' n=60
// expect: whitespace 2 42 17
// expect: scanset 2 'abc' 123
// expect: mismatch 0
// expect: suppressed 1 2 5
// expect: modifiers 7: 0.01 0.125 -32768 -128 99 -7 123
// expect: wide scan 1 'wide'
// expect: special 3 1 1 1
// expect: fscanf 5: 7 seven 7.5 3.14 q, eof next -1
// expect: truncated 9 '1234'
// expect: size 0 -> 8
// expect: wide field 501 last ' 1|'
// expect: long precision 502 ends 00
// expect: DBL_MAX %f has 316 characters, starts 179769313486
// expect: 1e+01|9.500000000000000000000000000000e+00
// expect: swprintf 22 '-1 wide narrow   2.2 z'
// expect: swprintf overflow 1
