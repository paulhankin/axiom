/* flags: --c-std=c99 */
/* A home-made printf: a formatter driven by <stdarg.h> into a buffer, with
   flags (- + space # 0), field width and precision (also as '*'), length
   modifiers (hh h l ll z j t), the conversions d i u o x X c s p n % and
   f e g for doubles (written from scratch with an integer-based decimal
   conversion good to 17 digits), plus a vprintf-style layer, a wrapper
   that forwards its va_list, va_copy to format twice, and an output sink
   chosen through a function pointer. */
#include "host.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef void (*sink_fn)(void *ctx, char c);

struct buf {
    char *p;
    size_t n, cap;
};

static void buf_put(void *ctx, char c) {
    struct buf *b = ctx;
    if (b->n + 1 < b->cap) b->p[b->n] = c;
    b->n++;
}

struct counter { size_t count; unsigned sum; };
static void count_put(void *ctx, char c) {
    struct counter *k = ctx;
    k->count++;
    k->sum = k->sum * 31 + (unsigned char)c;
}

struct spec {
    bool left, plus, space, alt, zero;
    int width, prec;   /* prec < 0: none */
    char len;          /* 0, 'H' (hh), 'h', 'l', 'L' (ll), 'z', 'j', 't' */
    char conv;
};

struct out {
    sink_fn put;
    void *ctx;
    int written;
};

static void emit(struct out *o, char c) {
    o->put(o->ctx, c);
    o->written++;
}

static void emit_n(struct out *o, char c, int n) {
    while (n-- > 0) emit(o, c);
}

/* writes prefix, zeros, digits with padding per spec */
static void emit_field(struct out *o, const struct spec *s, const char *prefix, int zeros, const char *body, int blen) {
    int plen = 0;
    while (prefix[plen]) plen++;
    int total = plen + zeros + blen;
    int pad = s->width > total ? s->width - total : 0;
    if (!s->left && !s->zero) emit_n(o, ' ', pad);
    for (int i = 0; i < plen; i++) emit(o, prefix[i]);
    if (!s->left && s->zero) emit_n(o, '0', pad);
    emit_n(o, '0', zeros);
    for (int i = 0; i < blen; i++) emit(o, body[i]);
    if (s->left) emit_n(o, ' ', pad);
}

static void fmt_int(struct out *o, struct spec s, uintmax_t v, bool neg) {
    char digits[72];
    int n = 0;
    unsigned base = s.conv == 'o' ? 8 : (s.conv == 'x' || s.conv == 'X' || s.conv == 'p') ? 16 : 10;
    const char *dset = s.conv == 'X' ? "0123456789ABCDEF" : "0123456789abcdef";
    char tmp[72];
    uintmax_t u = v;
    while (u) { tmp[n++] = dset[u % base]; u /= base; }
    int nd = n;
    for (int i = 0; i < nd; i++) digits[i] = tmp[nd - 1 - i];
    int zeros = 0;
    if (s.prec >= 0) {
        s.zero = false;
        if (s.prec > nd) zeros = s.prec - nd;
    } else if (nd == 0) {
        zeros = 1;
    }
    char prefix[4] = { 0 };
    int pl = 0;
    if (s.conv == 'd' || s.conv == 'i') {
        if (neg) prefix[pl++] = '-';
        else if (s.plus) prefix[pl++] = '+';
        else if (s.space) prefix[pl++] = ' ';
    }
    if (s.alt && s.conv == 'o' && zeros == 0 && (nd == 0 || digits[0] != '0')) zeros = 1;
    if ((s.alt && v != 0 && (s.conv == 'x' || s.conv == 'X')) || s.conv == 'p') {
        prefix[pl++] = '0';
        prefix[pl++] = s.conv == 'X' ? 'X' : 'x';
    }
    if (s.left) s.zero = false;
    emit_field(o, &s, prefix, zeros, digits, nd);
}

/* decimal digits of a non-negative double: an integer part and `prec`
   fraction digits, rounded half up on the 17-digit decimal expansion */
static int dbl_to_fixed(double x, int prec, char *out) {
    /* scale into an integer of up to 19 digits plus a power of ten */
    char dg[400];
    int nd = 0, int_digits;
    if (x == 0) {
        dg[nd++] = '0';
        int_digits = 1;
    } else {
        int e10 = 0;
        double y = x;
        while (y >= 1e17) { y /= 10; e10++; }
        while (y < 1e16) { y *= 10; e10--; }
        uint64_t m = (uint64_t)(y + 0.5);
        if (m >= 100000000000000000ull) { m /= 10; e10++; }
        char t[20];
        int k = 0;
        while (m) { t[k++] = (char)('0' + m % 10); m /= 10; }
        for (int i = k - 1; i >= 0; i--) dg[nd++] = t[i];
        int_digits = nd + e10;
        if (int_digits <= 0) {
            /* leading zeros */
            int z = 1 - int_digits;
            for (int i = nd - 1; i >= 0; i--) dg[i + z] = dg[i];
            for (int i = 0; i < z; i++) dg[i] = '0';
            nd += z;
            int_digits = 1;
        }
    }
    while (nd < int_digits + prec + 1) dg[nd++] = '0';
    /* round at position int_digits + prec */
    int cut = int_digits + prec;
    if (dg[cut] >= '5') {
        int i = cut - 1;
        while (i >= 0 && dg[i] == '9') dg[i--] = '0';
        if (i >= 0) dg[i]++;
        else {
            for (int j = cut; j > 0; j--) dg[j] = dg[j - 1];
            dg[0] = '1';
            int_digits++;
            cut++;
        }
    }
    int n = 0;
    for (int i = 0; i < int_digits; i++) out[n++] = dg[i];
    if (prec > 0) {
        out[n++] = '.';
        for (int i = 0; i < prec; i++) out[n++] = dg[int_digits + i];
    }
    return n;
}

static int dbl_exp10(double x) {
    if (x == 0) return 0;
    int e = 0;
    while (x >= 10) { x /= 10; e++; }
    while (x < 1) { x *= 10; e--; }
    return e;
}

static void fmt_float(struct out *o, struct spec s, double x) {
    char body[420];
    int n = 0;
    bool neg = x < 0 || (x == 0 && 1 / x < 0);
    if (neg) x = -x;
    int prec = s.prec < 0 ? 6 : s.prec;
    char c = s.conv;
    if (x != x || x > 1.7976931348623157e308) {
        const char *t = x != x ? "nan" : "inf";
        for (int i = 0; i < 3; i++) body[n++] = t[i];
        s.zero = false;
    } else if (c == 'f') {
        n = dbl_to_fixed(x, prec, body);
        if (s.alt && prec == 0) body[n++] = '.';
    } else {
        bool g = c == 'g';
        if (g && prec == 0) prec = 1;
        int e = dbl_exp10(x);
        /* the rounded exponent: round to prec significant digits first */
        double scaled = x;
        for (int i = 0; i < e; i++) scaled /= 10;
        for (int i = 0; i > e; i--) scaled *= 10;
        char tmp[64];
        int sig = g ? prec : prec + 1;
        int tn = dbl_to_fixed(scaled, sig - 1, tmp);
        if (tn > 0 && tmp[0] == '1' && tmp[1] == '0' && (tn == 2 || tmp[2] == '.')) {
            e++;
            scaled /= 10;
            tn = dbl_to_fixed(scaled, sig - 1, tmp);
        }
        if (g && e >= -4 && e < prec) {
            n = dbl_to_fixed(x, prec - 1 - e, body);
            if (!s.alt) {
                bool point = false;
                for (int i = 0; i < n; i++) point |= body[i] == '.';
                if (point) {
                    while (body[n - 1] == '0') n--;
                    if (body[n - 1] == '.') n--;
                }
            }
        } else {
            for (int i = 0; i < tn; i++) body[n++] = tmp[i];
            if (g && !s.alt) {
                bool point = false;
                for (int i = 0; i < n; i++) point |= body[i] == '.';
                if (point) {
                    while (body[n - 1] == '0') n--;
                    if (body[n - 1] == '.') n--;
                }
            }
            body[n++] = 'e';
            body[n++] = e < 0 ? '-' : '+';
            int a = e < 0 ? -e : e;
            if (a >= 100) body[n++] = (char)('0' + a / 100);
            body[n++] = (char)('0' + a / 10 % 10);
            body[n++] = (char)('0' + a % 10);
        }
    }
    char prefix[2] = { 0 };
    if (neg) prefix[0] = '-';
    else if (s.plus) prefix[0] = '+';
    else if (s.space) prefix[0] = ' ';
    if (s.left) s.zero = false;
    s.prec = -1;
    emit_field(o, &s, prefix, 0, body, n);
}

static int format_v(sink_fn put, void *ctx, const char *f, va_list ap) {
    struct out o = { put, ctx, 0 };
    while (*f) {
        if (*f != '%') { emit(&o, *f++); continue; }
        f++;
        struct spec s = { .prec = -1 };
        for (;; f++) {
            if (*f == '-') s.left = true;
            else if (*f == '+') s.plus = true;
            else if (*f == ' ') s.space = true;
            else if (*f == '#') s.alt = true;
            else if (*f == '0') s.zero = true;
            else break;
        }
        if (*f == '*') {
            s.width = va_arg(ap, int);
            if (s.width < 0) { s.left = true; s.width = -s.width; }
            f++;
        } else {
            while (*f >= '0' && *f <= '9') s.width = s.width * 10 + (*f++ - '0');
        }
        if (*f == '.') {
            f++;
            s.prec = 0;
            if (*f == '*') {
                s.prec = va_arg(ap, int);
                f++;
            } else {
                while (*f >= '0' && *f <= '9') s.prec = s.prec * 10 + (*f++ - '0');
            }
        }
        switch (*f) {
        case 'h':
            f++;
            if (*f == 'h') { s.len = 'H'; f++; } else s.len = 'h';
            break;
        case 'l':
            f++;
            if (*f == 'l') { s.len = 'L'; f++; } else s.len = 'l';
            break;
        case 'z': case 'j': case 't':
            s.len = *f++;
            break;
        }
        s.conv = *f;
        if (!*f) break;
        f++;
        switch (s.conv) {
        case 'd': case 'i': {
            intmax_t v;
            switch (s.len) {
            case 'H': v = (signed char)va_arg(ap, int); break;
            case 'h': v = (short)va_arg(ap, int); break;
            case 'l': v = va_arg(ap, long); break;
            case 'L': v = va_arg(ap, long long); break;
            case 'j': v = va_arg(ap, intmax_t); break;
            case 'z': v = (intmax_t)va_arg(ap, size_t); break;
            case 't': v = va_arg(ap, ptrdiff_t); break;
            default: v = va_arg(ap, int); break;
            }
            fmt_int(&o, s, v < 0 ? (uintmax_t)0 - (uintmax_t)v : (uintmax_t)v, v < 0);
            break;
        }
        case 'u': case 'o': case 'x': case 'X': {
            uintmax_t v;
            switch (s.len) {
            case 'H': v = (unsigned char)va_arg(ap, unsigned); break;
            case 'h': v = (unsigned short)va_arg(ap, unsigned); break;
            case 'l': v = va_arg(ap, unsigned long); break;
            case 'L': v = va_arg(ap, unsigned long long); break;
            case 'j': v = va_arg(ap, uintmax_t); break;
            case 'z': v = va_arg(ap, size_t); break;
            case 't': v = (uintmax_t)va_arg(ap, ptrdiff_t); break;
            default: v = va_arg(ap, unsigned); break;
            }
            fmt_int(&o, s, v, false);
            break;
        }
        case 'p': {
            void *p = va_arg(ap, void *);
            /* addresses differ between targets: print whether it is null */
            fmt_int(&o, s, p != NULL, false);
            break;
        }
        case 'c': {
            char c = (char)va_arg(ap, int);
            s.prec = -1;
            emit_field(&o, &s, "", 0, &c, 1);
            break;
        }
        case 's': {
            const char *str = va_arg(ap, const char *);
            if (!str) str = "(null)";
            int n = 0;
            while (str[n] && (s.prec < 0 || n < s.prec)) n++;
            s.zero = false;
            emit_field(&o, &s, "", 0, str, n);
            break;
        }
        case 'f': case 'e': case 'g':
            fmt_float(&o, s, va_arg(ap, double));
            break;
        case 'n':
            *va_arg(ap, int *) = o.written;
            break;
        case '%':
            emit(&o, '%');
            break;
        default:
            emit(&o, '%');
            emit(&o, s.conv);
            break;
        }
    }
    return o.written;
}

static char line[256];

static int sformat(char *dst, size_t cap, const char *f, ...) {
    struct buf b = { dst, 0, cap };
    va_list ap;
    va_start(ap, f);
    int n = format_v(buf_put, &b, f, ap);
    va_end(ap);
    if (cap) dst[b.n < cap ? b.n : cap - 1] = 0;
    return n;
}

static unsigned total_hash;

/* formats into the line, prints it, and also counts it with a copy of the list */
static void show(const char *f, ...) {
    va_list ap, aq;
    va_start(ap, f);
    va_copy(aq, ap);
    struct buf b = { line, 0, sizeof line };
    int n = format_v(buf_put, &b, f, ap);
    struct counter k = { 0, 0 };
    int m = format_v(count_put, &k, f, aq);
    va_end(aq);
    va_end(ap);
    if (n != m || (size_t)n != k.count) print_str("length mismatch", 15);
    total_hash = total_hash * 7 + k.sum;
    print_str(line, b.n < sizeof line ? b.n : sizeof line - 1);
}

static int forward(const char *prefix, const char *f, va_list ap) {
    char tmp[128];
    struct buf b = { tmp, 0, sizeof tmp };
    for (const char *p = prefix; *p; p++) buf_put(&b, *p);
    int n = format_v(buf_put, &b, f, ap);
    print_str(tmp, b.n);
    return n;
}

static int logf_(const char *f, ...) {
    va_list ap;
    va_start(ap, f);
    int n = forward("[log] ", f, ap);
    va_end(ap);
    return n;
}

int main(void) {
    show("plain text, %% sign");
    show("[%d] [%5d] [%-5d] [%05d] [%+d] [% d]", 42, 42, 42, 42, 42, 42);
    show("[%d] [%i] [%.3d] [%8.3d] [%-8.3d|] [%.0d]", -7, -2147483647 - 1, 5, -5, 5, 0);
    show("[%u] [%x] [%X] [%#x] [%#o] [%o] [%#X]", 4294967295u, 3735928559u, 48879u, 255u, 8u, 0u, 0u);
    show("[%hhd] [%hhu] [%hd] [%hu]", 300, 300, 70000, 70000);
    show("[%ld] [%lu] [%lld] [%llu] [%llx]", -123456789L, 123456789UL, -9223372036854775807LL - 1,
         18446744073709551615ULL, 0x123456789abcdefULL);
    show("[%zu] [%td] [%jd] [%ju]", sizeof(int[10]), (ptrdiff_t)-3, (intmax_t)-99, (uintmax_t)99);
    show("[%c%c%c] [%3c] [%-3c|]", 'a', 'b', 'c', 'x', 'y');
    show("[%s] [%10s] [%-10s|] [%.2s] [%*s] [%-*s|] [%.*s]", "hello", "right", "left", "truncate", 6, "w", 6, "w", 3,
         "precision");
    show("[%s] [%p] [%p]", (char *)NULL, (void *)0, (void *)line);
    show("[%f] [%.2f] [%10.3f] [%-10.1f|] [%+.0f] [%#.0f] [%010.2f]", 3.14159265, 2.675, -1.5, 0.05, 2.5, 3.0, -3.14159);
    show("[%e] [%.2e] [%E]", 123456.789, 0.000123456, 1.0);
    show("[%g] [%g] [%g] [%g] [%.3g] [%#g] [%g]", 100000.0, 1000000.0, 0.0001, 0.00001, 3.14159, 1.0, 0.0);
    show("[%f] [%e] [%g]", 1e300 * 1e10, -(1e300 * 1e10), 0.0 / 0.0 == 0 ? 1.0 : 2.5);
    show("[%.10f] [%.15g] [%.17g]", 1.0 / 3, 2.0 / 3, 0.1);
    show("[%5.1f%%] [%x%X] [%%%d%%]", 99.44, 10, 11, 7);
    int pos = 0;
    show("abc%ndef [%d]", &pos, 0);
    print_i64(pos);
    show("[%*d] [%-*d] [%.*f]", 8, -12, -8, 34, 3, 2.0 / 3);
    show("[%f] [%f] [%.1f] [%.1f]", 0.5, 1e-7, 0.05, 0.15);
    show("[%g] [%g] [%g]", 123456789.0, 1.5e-5, 9.9999999e5);
    show("[%q] unknown conversion");

    char small[8];
    int n = sformat(small, sizeof small, "%d-%d-%d", 1234, 5678, 9);
    print_i64(n);
    print_str(small, 7);
    n = logf_("%s=%d (%.2f%%)", "ratio", 3, 37.5);
    print_i64(n);
    n = logf_("%#08x|%-6s|%c", 0xbeef, "ok", '!');
    print_i64(n);

    /* the same through many arguments */
    show("%d %d %d %d %d %d %d %d %d %d %d %d %f %f %f %f %f %f %f %f %f %s",
         1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, "end");
    print_u64(total_hash);
    return 0;
}

/* expect: plain text, % sign */
/* expect: [42] [   42] [42   ] [00042] [+42] [ 42] */
/* expect: [-7] [-2147483648] [005] [    -005] [005     |] [] */
/* expect: [4294967295] [deadbeef] [BEEF] [0xff] [010] [0] [0] */
/* expect: [44] [44] [4464] [4464] */
/* expect: [-123456789] [123456789] [-9223372036854775808] [18446744073709551615] [123456789abcdef] */
/* expect: [40] [-3] [-99] [99] */
/* expect: [abc] [  x] [y  |] */
/* expect: [hello] [     right] [left      |] [tr] [     w] [w     |] [pre] */
/* expect: [(null)] [0x0] [0x1] */
/* expect: [3.141593] [2.68] [    -1.500] [0.1       |] [+3] [3.] [-000003.14] */
/* expect: [1.234568e+05] [1.23e-04] [%E] */
/* expect: [100000] [1e+06] [0.0001] [1e-05] [3.14] [1.00000] [0] */
/* expect: [inf] [-inf] [2.5] */
/* expect: [0.3333333333] [0.666666666666666] [0.1] */
/* expect: [ 99.4%] [aB] [%7%] */
/* expect: abcdef [0] */
/* expect: 3 */
/* expect: [     -12] [34      ] [0.667] */
/* expect: [0.500000] [0.000000] [0.1] [0.2] */
/* expect: [1.23457e+08] [1.5e-05] [1e+06] */
/* expect: [%q] unknown conversion */
/* expect: 11 */
/* expect: 1234-56 */
/* expect: [log] ratio=3 (37.50%) */
/* expect: 16 */
/* expect: [log] 0x00beef|ok    |! */
/* expect: 17 */
/* expect: 1 2 3 4 5 6 7 8 9 10 11 12 0.500000 1.500000 2.500000 3.500000 4.500000 5.500000 6.500000 7.500000 8.500000 end */
/* expect: 2694104657 */
