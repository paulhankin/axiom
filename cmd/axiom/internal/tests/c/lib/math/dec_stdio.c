/* The decimal floating types' formatted input and output (C23 7.23.6: the
   length modifiers H, D and DD with a e f g and their upper case: %a shows
   the coefficient and the quantum exponent exactly), strtod32 and its kin
   (correctly rounded in the decimal rounding direction, the quantum as
   written), wcstod64, strfromd32 ..., scanf's H, D and DD, round trips. */
/* flags: --c-std=c23 */
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

/* a value as %a writes it: its coefficient and quantum */
#define dec_show(x) _Generic((x), _Decimal32: show32, _Decimal64: show64, _Decimal128: show128)(x)
static void show32(_Decimal32 x) {
    printf("%Ha", x);
}
static void show64(_Decimal64 x) {
    printf("%Da", x);
}
static void show128(_Decimal128 x) {
    printf("%DDa", x);
}

static void nl(void) {
    printf("\n");
}

int main(void) {
    char buf[200], *end;
    /* %a: as the value is, with its quantum */
    printf("%Da|%Da|%Da|%Da|%Da|%Da", 1.20dd, 1.2E+3dd, 1200.dd, 0.000001dd, 1E-7dd, -0.0E-2dd);
/* expect: 1.20|1.2e+03|1200|0.000001|1e-07|-0.000 */
    nl();
    printf("%HA|%DDa|%.3Da|%.1Da|%#Da|%10.2Da|%-8Ha|", 1.5E-10df, 1.000dl, 123456.dd, 9.99dd, 5.dd, 3.14159dd, 2.5df);
/* expect: 1.5E-10|1.000|1.23e+05|1e+01|5.|       3.1|2.5     | */
    nl();
    /* e f g: the value's digits rounded (half to even), as for double */
    printf("%De|%.2De|%Df|%.0Df|%Dg|%Dg|%Dg|%#.3Dg", 1.20dd, 2.345dd, 1.5dd, 2.5dd, 100000.dd, 1000000.dd, 0.0001dd, 1.dd);
/* expect: 1.200000e+00|2.34e+00|1.500000|2|100000|1e+06|0.0001|1.00 */
    nl();
    printf("%.40Df|%.30DDe", 1E-30dd, 1.0dl / 3);
/* expect: 0.0000000000000000000000000000010000000000|3.333333333333333333333333333333e-01 */
    nl();
    printf("%Hf|%+De|% Dg|%08.2Df|%DF|%DG", DEC32_MAX, 1E-398dd, 2.5dd, -3.14159dd, (_Decimal64)DEC_INFINITY,
           -__builtin_axiom_dec(64, 0, 0x7c00000000000000));
/* expect: 9999999000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000.000000|+1.000000e-398| 2.5|-0003.14|INF|-NAN */
    nl();
    /* strtod32 ...: the quantum as written, rounding, ranges, the subject sequence */
    dec_show(strtod64("  +12.3450e2xyz", &end));
    printf(" [%s]", end);
/* expect: 1234.50 [xyz] */
    nl();
    dec_show(strtod32("0.000", &end));
    printf(" ");
    dec_show(strtod32("1.234567890", &end));
    printf(" ");
    dec_show(strtod128("-INFinity", &end));
    printf(" ");
    dec_show(strtod64("nan(12)", &end));
    printf(" ");
    dec_show(strtod64("0x1p3", &end));
    printf(" [%s]", end);
/* expect: 0.000 1.234568 -inf nan 0 [x1p3] */
    nl();
    errno = 0;
    dec_show(strtod32("1e97", &end));
    printf(" %d ", errno == ERANGE);
    errno = 0;
    dec_show(strtod32("1e-102", &end));
    printf(" %d ", errno == ERANGE);
    errno = 0;
    dec_show(strtod32("1.000e-101", &end));
    printf(" %d", errno == ERANGE);
/* expect: inf 1 0e-101 1 1e-101 0 */
    nl();
    fe_dec_setround(FE_DEC_UPWARD);
    dec_show(strtod32("1.0000001", &end));
    printf(" ");
    fe_dec_setround(FE_DEC_TOWARDZERO);
    dec_show(strtod32("-1.9999999999", &end));
    printf(" ");
    dec_show(strtod32("1e97", &end));
    fe_dec_setround(FE_DEC_TONEAREST);
/* expect: 1.000001 -1.999999 9.999999e+96 */
    nl();
    end = 0;
    dec_show(strtod64("", &end));
    printf(" %d", end != 0);
/* expect: 0 1 */
    nl();
    /* wide strings */
    {
        wchar_t *wend;
        dec_show(wcstod64(L"  -0.50e1 rest", &wend));
        printf(" [%ls]", wend);
/* expect: -5.0 [ rest] */
        nl();
    }
    /* strfromd */
    strfromd32(buf, sizeof buf, "%a", 1.500df);
    printf("%s|", buf);
    strfromd64(buf, sizeof buf, "%.3e", 1.0dd / 7);
    printf("%s|", buf);
    printf("%d|", strfromd128(buf, 4, "%f", 12.5dl));
    printf("%s|%d", buf, strfromd64(buf, sizeof buf, "%d", 1.dd));
/* expect: 1.500|1.429e-01|9|12.|-1 */
    nl();
    /* scanf */
    {
        _Decimal32 a;
        _Decimal64 b;
        _Decimal128 c;
        int n = sscanf("2.50 -1e-3 7.000000000000000000000000000000001E+6000 0x5", "%Hf %De %DDg", &a, &b, &c);
        printf("%d ", n);
        dec_show(a);
        printf(" ");
        dec_show(b);
        printf(" ");
        dec_show(c);
/* expect: 3 2.50 -0.001 7.000000000000000000000000000000001e+6000 */
        nl();
        n = sscanf("0x5", "%Da", &b);
        printf("%d ", n);
        dec_show(b);
/* expect: 1 0 */
        nl();
        n = sscanf("1.5", "%Hd", &a);
        printf("%d", n);
/* expect: 0 */
        nl();
    }
    /* round trips: %a's text read back is the same encoding */
    {
        static const _Decimal64 vals[] = {1.20dd, -7E-398dd, 9.999999999999999E384dd, 1.234567890123456E-100dd,
                                          0.0E5dd, 123456789E-5dd};
        int bad = 0;
        for (size_t i = 0; i < sizeof vals / sizeof vals[0]; i++) {
            _Decimal64 back;
            unsigned long long u1, u2;
            snprintf(buf, sizeof buf, "%Da", vals[i]);
            back = strtod64(buf, &end);
            memcpy(&u1, &vals[i], 8);
            memcpy(&u2, &back, 8);
            bad += u1 != u2 || *end != 0;
        }
        printf("%d", bad);
/* expect: 0 */
        nl();
    }
    return 0;
}
