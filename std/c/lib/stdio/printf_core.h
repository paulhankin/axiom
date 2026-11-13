/* printf_core.h - the formatted output engine (C17 7.21.6.1, and 7.29.2.1
   for the wide forms), one text for both: printf.c includes it with CH
   char and PF_NAME __axc_vprintf_core, wprintf.c with CH wchar_t and
   PF_WIDE 1. The output (struct __axc_out, impl.h) takes runs of CH.

   Everything of 7.21.6.1 and C23's additions: the flags - + space # 0, the
   field width and precision (or *), the length modifiers hh h l ll j z t L
   and C23's wN, wfN and H, D, DD (the decimal floating types, with a e f g
   and their upper case: fmtfloat.c), the conversions d i o u x X b B (C23) f F e E g G a
   A c s p n %. Characters of the format that are not a conversion are
   copied a run at a time. */

/* Writes n characters of the narrow text s (ASCII here) to the output */
static void PF_TEXT(struct __axc_out *o, const char *s, size_t n) {
#if PF_WIDE
    CH w[64];
    while (n > 0) {
        size_t k = n < 64 ? n : 64, i;
        for (i = 0; i < k; i++)
            w[i] = (CH)(unsigned char)s[i];
        o->put(o, w, k);
        s += k;
        n -= k;
    }
#else
    o->put(o, s, n);
#endif
}

/* n copies of c */
static void PF_PAD(struct __axc_out *o, int c, size_t n) {
    CH w[32];
    size_t i;
    for (i = 0; i < 32 && i < n; i++)
        w[i] = (CH)c;
    while (n > 0) {
        size_t k = n < 32 ? n : 32;
        o->put(o, w, k);
        n -= k;
    }
}

/* A converted field: padding to the width, a prefix (a sign, 0x), zeros
   (from the 0 flag or the precision), then the body */
static void PF_FIELD(struct __axc_out *o, const char *prefix, size_t np, size_t zeros, const char *body, size_t nb,
                     int width, int left, int zero_pad) {
    size_t len = np + zeros + nb, pad = width > 0 && (size_t)width > len ? (size_t)width - len : 0;
    if (zero_pad && !left) {
        zeros += pad;
        pad = 0;
    }
    if (!left && pad)
        PF_PAD(o, ' ', pad);
    if (np)
        PF_TEXT(o, prefix, np);
    if (zeros)
        PF_PAD(o, '0', zeros);
    if (nb)
        PF_TEXT(o, body, nb);
    if (left && pad)
        PF_PAD(o, ' ', pad);
}

#ifndef PF_ZERO_STATE
/* the initial conversion state (7.29.6p2: a zero-valued mbstate_t) */
#define PF_ZERO_STATE(st) \
    do { \
        unsigned char *z_ = (unsigned char *)(st); \
        size_t k_; \
        for (k_ = 0; k_ < sizeof(mbstate_t); k_++) \
            z_[k_] = 0; \
    } while (0)
/* a wint_t argument (promoted to int if narrower: 6.5.2.2p6) */
#if WINT_MAX < INT_MAX
#define PF_WINT_ARG(ap) ((wint_t)va_arg(ap, int))
#else
#define PF_WINT_ARG(ap) (va_arg(ap, wint_t))
#endif
#endif

/* The bits of the type a length modifier names, for an integer conversion
   (0: int, after the promotions) */
#define BITS_OF(t) ((int)(sizeof(t) * CHAR_BIT))

int PF_NAME(struct __axc_out *o, const CH *fmt, va_list ap) {
    const CH *p = fmt;
    for (;;) {
        const CH *q = p;
        int left = 0, plus = 0, space = 0, alt = 0, zero = 0, width = 0, prec = -1, bits, conv, ldbl = 0, dfmt = 0;
        int wide_char = 0, lm = 0; /* the modifier's letter (w: wN), for %n */
        while (*q != 0 && *q != '%')
            q++;
        if (q > p)
            o->put(o, p, (size_t)(q - p));
        if (*q == 0)
            break;
        p = q + 1;
        /* the flags, in any order */
        for (;; p++) {
            if (*p == '-')
                left = 1;
            else if (*p == '+')
                plus = 1;
            else if (*p == ' ')
                space = 1;
            else if (*p == '#')
                alt = 1;
            else if (*p == '0')
                zero = 1;
            else
                break;
        }
        /* the width: a negative * is the - flag and the width (7.21.6.1p5) */
        if (*p == '*') {
            width = va_arg(ap, int);
            if (width < 0) {
                left = 1;
                width = width == INT_MIN ? INT_MAX : -width;
            }
            p++;
        } else {
            while (*p >= '0' && *p <= '9') {
                width = width > (INT_MAX - 9) / 10 ? INT_MAX : width * 10 + (*p - '0');
                p++;
            }
        }
        /* the precision: a negative * is as if there were none */
        if (*p == '.') {
            p++;
            if (*p == '*') {
                prec = va_arg(ap, int);
                if (prec < 0)
                    prec = -1;
                p++;
            } else {
                prec = 0;
                while (*p >= '0' && *p <= '9') {
                    prec = prec > (INT_MAX - 9) / 10 ? INT_MAX : prec * 10 + (*p - '0');
                    p++;
                }
            }
        }
        /* the length modifier, as the width of the argument's type */
        bits = 0;
        dfmt = 0;
        switch (*p) {
        case 'H':
            /* C23: _Decimal32, _Decimal64 (D), _Decimal128 (DD) */
            dfmt = 1;
            p++;
            break;
        case 'D':
            dfmt = p[1] == 'D' ? 3 : 2;
            p += dfmt == 3 ? 2 : 1;
            break;
        case 'h':
            if (p[1] == 'h') {
                bits = CHAR_BIT;
                lm = 'H';
                p += 2;
            } else {
                bits = BITS_OF(short);
                lm = 'h';
                p++;
            }
            break;
        case 'l':
            if (p[1] == 'l') {
                bits = BITS_OF(long long);
                lm = 'L';
                p += 2;
            } else {
                bits = BITS_OF(long);
                lm = 'l';
                wide_char = 1;
                p++;
            }
            break;
        case 'j':
            bits = BITS_OF(intmax_t);
            lm = 'j';
            p++;
            break;
        case 'z':
            bits = BITS_OF(size_t);
            lm = 'z';
            p++;
            break;
        case 't':
            bits = BITS_OF(ptrdiff_t);
            lm = 't';
            p++;
            break;
        case 'L':
            ldbl = 1;
            p++;
            break;
        case 'w': {
            /* C23: wN (intN_t), wfN (int_fastN_t), N one of 8 16 32 64 */
            int fast = p[1] == 'f', n = 0;
            const CH *r = p + 1 + fast;
            while (*r >= '0' && *r <= '9' && n < 1000)
                n = n * 10 + (*r++ - '0');
            if (n == 8)
                bits = fast ? BITS_OF(int_fast8_t) : 8;
            else if (n == 16)
                bits = fast ? BITS_OF(int_fast16_t) : 16;
            else if (n == 32)
                bits = fast ? BITS_OF(int_fast32_t) : 32;
            else if (n == 64)
                bits = fast ? BITS_OF(int_fast64_t) : 64;
            else
                return -1; /* (a width no type has) */
            lm = 'w';
            p = r;
            break;
        }
        default:
            break;
        }
        conv = *p;
        if (conv == 0)
            return -1; /* (the format ends inside a conversion) */
        if (dfmt != 0 && (conv | 0x20) != 'a' && (conv | 0x20) != 'e' && (conv | 0x20) != 'f' && (conv | 0x20) != 'g')
            return -1; /* (H, D and DD take only the floating conversions) */
        p++;
        switch (conv) {
        case 'd':
        case 'i':
        case 'o':
        case 'u':
        case 'x':
        case 'X':
        case 'b':
        case 'B': {
            /* 7.21.6.1p8: at least prec digits (default 1), none for a zero
               with precision 0; # gives o a leading 0, x 0x, b 0b */
            unsigned __axiom_int64 u;
            char buf[72], prefix[4];
            size_t nb = 0, np = 0, nz = 0;
            int base = conv == 'o' ? 8 : conv == 'x' || conv == 'X' ? 16 : conv == 'b' || conv == 'B' ? 2 : 10;
            const char *dg = conv == 'X' ? "0123456789ABCDEF" : "0123456789abcdef";
            int sgn = conv == 'd' || conv == 'i', neg = 0;
            int ab = bits == 0 ? BITS_OF(int) : bits;
            /* the argument: of the type, after the default promotions */
            if (ab <= BITS_OF(int))
                u = sgn ? (unsigned __axiom_int64)(__axiom_int64)va_arg(ap, int) : (unsigned __axiom_int64)va_arg(ap, unsigned int);
            else if (ab <= BITS_OF(long))
                u = sgn ? (unsigned __axiom_int64)(__axiom_int64)va_arg(ap, long) : (unsigned __axiom_int64)va_arg(ap, unsigned long);
            else
                u = (unsigned __axiom_int64)va_arg(ap, unsigned __axiom_int64);
            /* converted to the type (hh, h: 7.21.6.1p7) */
            if (ab < 64) {
                unsigned __axiom_int64 m = ((unsigned __axiom_int64)1 << ab) - 1;
                u &= m;
                if (sgn && (u >> (ab - 1)) & 1)
                    u |= ~m;
            }
            if (sgn && (__axiom_int64)u < 0) {
                neg = 1;
                u = 0 - u;
            }
            if (base == 10) {
                while (u != 0) {
                    buf[sizeof buf - 1 - nb++] = (char)('0' + u % 10);
                    u /= 10;
                }
            } else {
                int sh = base == 16 ? 4 : base == 8 ? 3 : 1;
                unsigned __axiom_int64 v = u;
                while (v != 0) {
                    buf[sizeof buf - 1 - nb++] = dg[v & (unsigned)(base - 1)];
                    v >>= sh;
                }
            }
            if (prec < 0) {
                if (nb == 0)
                    nz = 1;
            } else if ((size_t)prec > nb) {
                nz = (size_t)prec - nb;
            }
            if (sgn) {
                if (neg)
                    prefix[np++] = '-';
                else if (plus)
                    prefix[np++] = '+';
                else if (space)
                    prefix[np++] = ' ';
            } else if (alt && base == 8) {
                if (nz == 0 && (nb == 0 || buf[sizeof buf - nb] != '0'))
                    nz = 1;
            } else if (alt && u != 0 && (base == 16 || base == 2)) {
                prefix[np++] = '0';
                prefix[np++] = (char)(base == 16 ? conv : conv == 'B' ? 'B' : 'b');
            }
            PF_FIELD(o, prefix, np, nz, buf + sizeof buf - nb, nb, width, left, zero && prec < 0);
            break;
        }
        case 'f':
        case 'F':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
        case 'a':
        case 'A': {
            struct __axc_fp v;
            char buf[400], *big = NULL, prefix[2];
            size_t bigcap = 0, n, np = 0;
            int upper = conv >= 'A' && conv <= 'Z';
            if (dfmt == 1) {
                __axiom_decimal32 x = va_arg(ap, __axiom_decimal32);
                __axc_fp_dec(&v, &x, 4);
            } else if (dfmt == 2) {
                __axiom_decimal64 x = va_arg(ap, __axiom_decimal64);
                __axc_fp_dec(&v, &x, 8);
            } else if (dfmt == 3) {
                __axiom_decimal128 x = va_arg(ap, __axiom_decimal128);
                __axc_fp_dec(&v, &x, 16);
            } else if (ldbl)
                __axc_fp_long_double(&v, va_arg(ap, long double));
            else
                __axc_fp_double(&v, va_arg(ap, double));
            if (v.neg)
                prefix[np++] = '-';
            else if (plus)
                prefix[np++] = '+';
            else if (space)
                prefix[np++] = ' ';
            if (v.kind >= 2) {
                /* infinity, NaN: inf, nan (INF, NAN), not padded with zeros */
                PF_FIELD(o, prefix, np, 0, v.kind == 2 ? (upper ? "INF" : "inf") : (upper ? "NAN" : "nan"), 3,
                         width, left, 0);
                break;
            }
            n = __axc_fmt_fp(&v, conv, prec, alt, buf, sizeof buf, &big, &bigcap);
            if (n == 0) {
                o->failed = 1;
#ifdef ENOMEM
                SET_ERRNO(ENOMEM);
#endif
                break;
            }
            {
                const char *body = big != NULL ? big : buf;
                if ((conv | 0x20) == 'a' && !v.dec) {
                    /* the zeros of the 0 flag go after 0x */
                    char pre[4];
                    size_t k;
                    for (k = 0; k < np; k++)
                        pre[k] = prefix[k];
                    pre[np] = body[0];
                    pre[np + 1] = body[1];
                    PF_FIELD(o, pre, np + 2, 0, body + 2, n - 2, width, left, zero);
                } else {
                    PF_FIELD(o, prefix, np, 0, body, n, width, left, zero);
                }
            }
            if (big != NULL)
                __axc_sys_free(big, bigcap);
            break;
        }
        case 'c': {
#if PF_WIDE
            /* a byte as btowc converts it, or a wide character (%lc) */
            CH c;
            if (wide_char) {
                c = (CH)PF_WINT_ARG(ap);
            } else {
                wint_t w = btowc(va_arg(ap, int));
                if (w == WEOF) {
                    o->failed = 1;
                    SET_ERRNO(EILSEQ);
                    return -1;
                }
                c = (CH)w;
            }
            {
                size_t pad = width > 1 ? (size_t)width - 1 : 0;
                if (!left && pad)
                    PF_PAD(o, ' ', pad);
                o->put(o, &c, 1);
                if (left && pad)
                    PF_PAD(o, ' ', pad);
            }
#else
            char mb[8];
            size_t n = 1;
            if (wide_char) {
                /* %lc: the wide character as wcrtomb converts it (7.21.6.1p8) */
                mbstate_t st;
                wchar_t w = (wchar_t)PF_WINT_ARG(ap);
                PF_ZERO_STATE(&st);
                n = wcrtomb(mb, w, &st);
                if (n == (size_t)-1) {
                    o->failed = 1;
                    return -1;
                }
            } else {
                mb[0] = (char)(unsigned char)va_arg(ap, int);
            }
            PF_FIELD(o, NULL, 0, 0, mb, n, width, left, 0);
#endif
            break;
        }
        case 's': {
            if (wide_char) {
                /* %ls */
                const wchar_t *s = va_arg(ap, const wchar_t *);
#if PF_WIDE
                size_t n = 0, pad;
                while ((prec < 0 || n < (size_t)prec) && s[n] != 0)
                    n++;
                pad = width > 0 && (size_t)width > n ? (size_t)width - n : 0;
                if (!left && pad)
                    PF_PAD(o, ' ', pad);
                o->put(o, s, n);
                if (left && pad)
                    PF_PAD(o, ' ', pad);
#else
                /* converted as wcrtomb does, no partial multibyte character
                   within the precision (7.21.6.1p8) */
                mbstate_t st;
                char mb[16];
                size_t total = 0, i, n, pad;
                PF_ZERO_STATE(&st);
                for (i = 0; s[i] != 0; i++) {
                    n = wcrtomb(mb, s[i], &st);
                    if (n == (size_t)-1) {
                        o->failed = 1;
                        return -1;
                    }
                    if (prec >= 0 && total + n > (size_t)prec)
                        break;
                    total += n;
                }
                pad = width > 0 && (size_t)width > total ? (size_t)width - total : 0;
                if (!left && pad)
                    PF_PAD(o, ' ', pad);
                {
                    size_t done = 0;
                    PF_ZERO_STATE(&st);
                    for (i = 0; done < total; i++) {
                        n = wcrtomb(mb, s[i], &st);
                        o->put(o, mb, n);
                        done += n;
                    }
                }
                if (left && pad)
                    PF_PAD(o, ' ', pad);
#endif
            } else {
                const char *s = va_arg(ap, const char *);
#if PF_WIDE
                /* a multibyte string, converted as mbrtowc does (7.29.2.1p8) */
                mbstate_t st;
                size_t count = 0, len = 0, pad, r, i;
                const char *t;
                wchar_t w;
                PF_ZERO_STATE(&st);
                while (s[len] != 0)
                    len++;
                for (t = s; *t != 0 && (prec < 0 || count < (size_t)prec); count++) {
                    r = mbrtowc(&w, t, len - (size_t)(t - s), &st);
                    if (r == (size_t)-1 || r == (size_t)-2) {
                        o->failed = 1;
                        SET_ERRNO(EILSEQ);
                        return -1;
                    }
                    t += r == 0 ? 1 : r;
                }
                pad = width > 0 && (size_t)width > count ? (size_t)width - count : 0;
                if (!left && pad)
                    PF_PAD(o, ' ', pad);
                PF_ZERO_STATE(&st);
                for (t = s, i = 0; i < count; i++) {
                    r = mbrtowc(&w, t, len - (size_t)(t - s), &st);
                    o->put(o, &w, 1);
                    t += r == 0 ? 1 : r;
                }
                if (left && pad)
                    PF_PAD(o, ' ', pad);
#else
                size_t n = 0;
                if (prec < 0) {
                    while (s[n] != 0)
                        n++;
                } else {
                    while (n < (size_t)prec && s[n] != 0)
                        n++;
                }
                if (width == 0) {
                    o->put(o, s, n);
                } else {
                    size_t pad = (size_t)width > n ? (size_t)width - n : 0;
                    if (!left && pad)
                        PF_PAD(o, ' ', pad);
                    o->put(o, s, n);
                    if (left && pad)
                        PF_PAD(o, ' ', pad);
                }
#endif
            }
            break;
        }
        case 'p': {
            /* as %#x of the pointer's bits: 0x followed by hexadecimal
               digits (0x0 for a null pointer), which %p of fscanf reads */
            unsigned __axiom_int64 u = (unsigned __axiom_int64)(size_t)va_arg(ap, void *);
            char buf[24];
            size_t nb = 0;
            do {
                buf[sizeof buf - 1 - nb++] = "0123456789abcdef"[u & 15];
                u >>= 4;
            } while (u != 0);
            PF_FIELD(o, "0x", 2, prec > 0 && (size_t)prec > nb ? (size_t)prec - nb : 0, buf + sizeof buf - nb, nb,
                     width, left, 0);
            break;
        }
        case 'n': {
            /* the characters written so far, in an object of the length
               modifier's type */
            void *dst = va_arg(ap, void *);
            size_t c = o->count;
            switch (lm) {
            case 'H':
                *(signed char *)dst = (signed char)c;
                break;
            case 'h':
                *(short *)dst = (short)c;
                break;
            case 'l':
                *(long *)dst = (long)c;
                break;
            case 'L':
                *(long long *)dst = (long long)c;
                break;
            case 'j':
                *(intmax_t *)dst = (intmax_t)c;
                break;
            case 'z':
                *(size_t *)dst = c;
                break;
            case 't':
                *(ptrdiff_t *)dst = (ptrdiff_t)c;
                break;
            case 'w':
                if (bits == 8)
                    *(int8_t *)dst = (int8_t)c;
                else if (bits == 16)
                    *(int16_t *)dst = (int16_t)c;
                else if (bits == 32)
                    *(int_least32_t *)dst = (int_least32_t)c;
                else
                    *(int64_t *)dst = (int64_t)c;
                break;
            default:
                *(int *)dst = (int)c;
                break;
            }
            break;
        }
        case '%':
            o->put(o, p - 1, 1);
            break;
        default:
            /* (not a conversion of the standard's: undefined; it fails) */
            return -1;
        }
        if (o->failed)
            return -1;
    }
    if (o->failed)
        return -1;
    if (o->count > INT_MAX) {
#ifdef EOVERFLOW
        SET_ERRNO(EOVERFLOW);
#endif
        return -1;
    }
    return (int)o->count;
}

#undef BITS_OF
