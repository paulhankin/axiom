/* __axc_strfrombits: strfromf16 and strfromf128's formatting (<stdlib.h>,
   ISO/IEC TS 18661-3 12.3): the value of the bytes at b (which 0
   binary16, 1 binary128) formatted as snprintf would format it with the
   format "%[.precision]conversion" (a conversion of a, A, e, E, f, F, g or
   G; another format writes nothing and the result is -1), by the standard
   I/O's formatting engine (fmtfloat.c), into at most n bytes at s; the
   length the whole result has. */
#include "impl.h"

int __axc_strfrombits(char *s, size_t n, const char *format, const unsigned char *b, int which) {
    struct __axc_fp v;
    char buf[400], *big = NULL;
    size_t bigcap = 0, len = 0, k, i = 1;
    const char *body;
    int prec = -1, conv, upper;
    char pre[1];
    size_t np = 0;
    if (format[0] != '%')
        return -1;
    if (format[i] == '.') {
        i++;
        prec = 0;
        while (format[i] >= '0' && format[i] <= '9') {
            if (prec > 100000000)
                return -1;
            prec = prec * 10 + (format[i++] - '0');
        }
    }
    conv = format[i];
    switch (conv) {
    case 'a': case 'A': case 'e': case 'E': case 'f': case 'F': case 'g': case 'G':
        break;
    default:
        return -1;
    }
    if (format[i + 1] != 0)
        return -1;
    upper = conv >= 'A' && conv <= 'Z';
    __axc_fp_bits(&v, b, which);
    if (v.neg)
        pre[np++] = '-';
    if (v.kind >= 2) {
        body = v.kind == 2 ? (upper ? "INF" : "inf") : (upper ? "NAN" : "nan");
        len = 3;
    } else {
        len = __axc_fmt_fp(&v, conv, prec, 0, buf, sizeof buf, &big, &bigcap);
        if (len == 0)
            return -1;
        body = big != NULL ? big : buf;
    }
    /* the sign, then the body; the terminating null within n */
    for (k = 0; k < np + len; k++) {
        if (k + 1 < n)
            s[k] = k < np ? pre[k] : body[k - np];
    }
    if (n > 0)
        s[np + len < n ? np + len : n - 1] = 0;
    if (big != NULL)
        __axc_sys_free(big, bigcap);
    return (int)(np + len);
}
