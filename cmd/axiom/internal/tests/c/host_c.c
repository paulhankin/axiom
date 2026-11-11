/* host_c.c - the output functions of host.h for the native C tests on the
   targets where the system's C compiler does not build host_native.c
   (Windows, macOS): a second translation unit of each test, compiled with
   it (and with its options) over the platform's C library. Valid in every
   standard and with every --c-int and --c-long: the library functions it
   calls take and return no int (their results are not used). */
#include "host.h"

int puts(const char *s);
int printf(const char *fmt, ...);
int sprintf(char *buf, const char *fmt, ...);
double strtod(const char *s, char **end);

static void put_u64(unsigned __axiom_int64 x, int neg) {
    char buf[24];
    char *p = buf + sizeof buf - 1;
    *p = 0;
    do {
        *--p = (char)('0' + (int)(x % 10));
        x /= 10;
    } while (x != 0);
    if (neg)
        *--p = '-';
    puts(p);
}

void print_i64(__axiom_int64 x) {
    if (x < 0)
        put_u64(0 - (unsigned __axiom_int64)x, 1);
    else
        put_u64((unsigned __axiom_int64)x, 0);
}

void print_u64(unsigned __axiom_int64 x) {
    put_u64(x, 0);
}

void print_bool(int b) {
    puts(b ? "true" : "false");
}

void print_str(const char *p, size_t n) {
    char buf[256];
    size_t k = 0;
    while (n-- > 0) {
        buf[k++] = *p++;
        if (k == sizeof buf - 1) {
            buf[k] = 0;
            printf("%s", buf);
            k = 0;
        }
    }
    buf[k] = 0;
    puts(buf);
}

/* fmt = "%.<digits><conv>" */
static void make_fmt(char *fmt, int digits, char conv) {
    char *f = fmt;
    *f++ = '%';
    *f++ = '.';
    if (digits >= 10)
        *f++ = (char)('0' + digits / 10);
    *f++ = (char)('0' + digits % 10);
    *f++ = conv;
    *f = 0;
}

/* The shortest of the %.<p><conv> forms that reads back as x. */
static void shortest(char *buf, double x, char conv) {
    char fmt[8];
    int p;
    for (p = 1; p <= 17; p++) {
        make_fmt(fmt, p, conv);
        sprintf(buf, fmt, x);
        if (strtod(buf, (char **)0) == x)
            return;
    }
}

/* As the WebAssembly host prints a number (JavaScript's Number#toString in
   the common cases). */
void print_f64(double x) {
    char buf[400];
    char *e;
    double a = x < 0 ? -x : x;
    if (x != x) {
        puts("NaN");
        return;
    }
    if (x != 0 && x * 0.5 == x) {
        puts(x > 0 ? "Infinity" : "-Infinity");
        return;
    }
    if (x == 0) {
        puts("0");
        return;
    }
    shortest(buf, x, 'g');
    for (e = buf; *e && *e != 'e'; e++) {
    }
    if (*e && a >= 1e-6 && a < 1e21) {
        char *d;
        char *z;
        shortest(buf, x, 'f');
        for (d = buf; *d && *d != '.'; d++) {
        }
        if (*d) {
            for (z = d; z[1]; z++) {
            }
            while (z > d && *z == '0')
                *z-- = 0;
            if (z == d)
                *z = 0;
        }
    } else if (*e) {
        /* the exponent as 1e+21 and 1.5e-7: a sign, no leading zeros */
        char *s = e + 1;
        char *o = e + 1;
        char sign = '+';
        if (*s == '+' || *s == '-')
            sign = *s++;
        while (*s == '0' && s[1])
            s++;
        *o++ = sign;
        while (*s)
            *o++ = *s++;
        *o = 0;
    }
    puts(buf);
}
