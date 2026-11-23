/* m_nan.c - nan, nanf, nanl (C17 7.12.11.2): a quiet NaN whose payload is
   the n-char-sequence read as an unsigned integer (decimal, or hexadecimal
   after 0x, or octal after 0, as strtoull reads it with base 0), reduced
   to the payload's bits (51 for double, 22 for float, 9 for _Float16); 0 for any other
   sequence. This is the library's reading of "NAN(n-char-sequence)" (7.22.1.3). */
#if __STDC_VERSION__ >= 199901L
static unsigned long long payload(const char *s)
{
    unsigned long long v = 0;
    unsigned base = 10, d;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X') && s[2] != 0) {
        base = 16;
        s += 2;
    } else if (s[0] == '0') {
        base = 8;
    }
    for (; *s; s++) {
        char c = *s;
        if (c >= '0' && c <= '9')
            d = (unsigned)(c - '0');
        else if (c >= 'a' && c <= 'f')
            d = (unsigned)(c - 'a') + 10;
        else if (c >= 'A' && c <= 'F')
            d = (unsigned)(c - 'A') + 10;
        else
            return 0;
        if (d >= base)
            return 0;
        v = v * base + d;
    }
    return v;
}

double nan(const char *tag)
{
    union { unsigned long long u; double d; } r;
    r.u = 0x7ff8000000000000ULL | (payload(tag) & 0x7ffffffffffffULL);
    return r.d;
}

float nanf(const char *tag)
{
    /* (unsigned long has 32 bits or more, and the float is its first 4
       bytes: every target is little-endian) */
    union { unsigned long u; float f; } r;
    r.u = 0x7fc00000UL | (unsigned long)(payload(tag) & 0x3fffffUL);
    return r.f;
}

long double nanl(const char *tag)
{
    return (long double)nan(tag);
}

/* C23 Annex H.11: binary16 and binary128, the payload's low bits (9 and
   111; the sequence's 64 at most) */
_Float16 nanf16(const char *tag)
{
    union { unsigned short u; _Float16 h; } r;
    r.u = (unsigned short)(0x7e00u | (unsigned)(payload(tag) & 0x1ffu));
    return r.h;
}

_Float128 nanf128(const char *tag)
{
    /* (the low word first: every target is little-endian) */
    union { unsigned long long u[2]; _Float128 q; } r;
    r.u[0] = payload(tag);
    r.u[1] = 0x7fff800000000000ULL;
    return r.q;
}
#else
typedef int __axc_nan_c89;
#endif
