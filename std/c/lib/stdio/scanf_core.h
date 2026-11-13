/* scanf_core.h - the formatted input engine (C17 7.21.6.2, and 7.29.2.2
   for the wide forms), one text for both: scanf.c includes it with CH
   char and SF_NAME __axc_vscanf_core, wscanf.c with CH wchar_t and
   SF_WIDE 1. The input (struct __axc_in, impl.h) gives characters one at
   a time and takes one back.

   An input item is the longest sequence of characters (at most the width)
   that is a matching sequence or a prefix of one (7.21.6.2p9): the
   character after it is the only one given back, so that a prefix that is
   not a whole matching sequence ("0x" for %x, "1e" for %f) is a matching
   failure. The result is the number of assignments, or EOF for an input
   failure before the first conversion (7.21.6.2p16). */

/* White space as isspace (iswspace) has it in the "C" locale */
#define SF_SPACE(c) ((c) == ' ' || ((c) >= '\t' && (c) <= '\r'))

int SF_NAME(struct __axc_in *in, const CH *fmt, va_list ap) {
    const CH *p = fmt;
    int assigned = 0, convs = 0, c;
#define GET() (c = in->get(in), c >= 0 ? (in->count++, c) : c)
#define UNGET() (c >= 0 ? (in->unget(in, c), in->count--) : 0)
    while (*p != 0) {
        int suppress = 0, conv, lm = 0, bits = 0;
        long width = -1;
        void *dst = NULL;
        if (SF_SPACE(*p)) {
            /* white space in the format: any amount in the input */
            while (SF_SPACE(*p))
                p++;
            while (GET() >= 0 && SF_SPACE(c))
                ;
            UNGET();
            continue;
        }
        if (*p != '%' || p[1] == '%') {
            /* an ordinary character, or %% (after white space) */
            int want = *p;
            if (*p == '%') {
                p++;
                while (GET() >= 0 && SF_SPACE(c))
                    ;
                UNGET();
            }
            p++;
            if (GET() < 0)
                goto input_failure;
            if (c != want) {
                UNGET();
                goto matching_failure;
            }
            continue;
        }
        p++;
        if (*p == '*') {
            suppress = 1;
            p++;
        }
        if (*p >= '1' && *p <= '9') {
            width = 0;
            while (*p >= '0' && *p <= '9') {
                if (width < 100000000L)
                    width = width * 10 + (*p - '0');
                p++;
            }
        }
        switch (*p) {
        case 'h':
            if (p[1] == 'h') {
                lm = 'H';
                p += 2;
            } else {
                lm = 'h';
                p++;
            }
            break;
        case 'l':
            if (p[1] == 'l') {
                lm = 'L';
                p += 2;
            } else {
                lm = 'l';
                p++;
            }
            break;
        case 'j':
        case 'z':
        case 't':
            lm = *p++;
            break;
        case 'L':
            lm = 'D';
            p++;
            break;
        case 'H':
            /* C23: _Decimal32, _Decimal64 (D), _Decimal128 (DD) */
            lm = '1';
            p++;
            break;
        case 'D':
            lm = p[1] == 'D' ? '3' : '2';
            p += lm == '3' ? 2 : 1;
            break;
        case 'w': {
            /* C23: wN, wfN */
            int fast = p[1] == 'f', n = 0;
            const CH *r = p + 1 + fast;
            while (*r >= '0' && *r <= '9' && n < 1000)
                n = n * 10 + (*r++ - '0');
            if (n == 8)
                bits = fast ? (int)(sizeof(int_fast8_t) * CHAR_BIT) : 8;
            else if (n == 16)
                bits = fast ? (int)(sizeof(int_fast16_t) * CHAR_BIT) : 16;
            else if (n == 32)
                bits = fast ? (int)(sizeof(int_fast32_t) * CHAR_BIT) : 32;
            else if (n == 64)
                bits = fast ? (int)(sizeof(int_fast64_t) * CHAR_BIT) : 64;
            else
                goto matching_failure;
            lm = 'w';
            p = r;
            break;
        }
        default:
            break;
        }
        conv = *p++;
        if (conv == 0)
            goto matching_failure;
        if (lm >= '1' && lm <= '3' && (conv | 0x20) != 'a' && (conv | 0x20) != 'e' && (conv | 0x20) != 'f' &&
            (conv | 0x20) != 'g')
            goto matching_failure; /* (H, D and DD take only the floating conversions) */
        if (!suppress && conv != 'n')
            dst = va_arg(ap, void *);
        if (conv != '[' && conv != 'c' && conv != 'n') {
            /* 7.21.6.2p8: input white space is skipped */
            while (GET() >= 0 && SF_SPACE(c))
                ;
            if (c < 0)
                goto input_failure;
            UNGET();
        }
        switch (conv) {
        case 'n': {
            /* the characters read so far; not an assignment (p12) */
            void *d = suppress ? NULL : va_arg(ap, void *);
            size_t v = in->count;
            if (d == NULL)
                break;
            switch (lm) {
            case 'H':
                *(signed char *)d = (signed char)v;
                break;
            case 'h':
                *(short *)d = (short)v;
                break;
            case 'l':
                *(long *)d = (long)v;
                break;
            case 'L':
                *(long long *)d = (long long)v;
                break;
            case 'j':
                *(intmax_t *)d = (intmax_t)v;
                break;
            case 'z':
                *(size_t *)d = v;
                break;
            case 't':
                *(ptrdiff_t *)d = (ptrdiff_t)v;
                break;
            case 'w':
                if (bits == 8)
                    *(int8_t *)d = (int8_t)v;
                else if (bits == 16)
                    *(int16_t *)d = (int16_t)v;
                else if (bits == 32)
                    *(int_least32_t *)d = (int_least32_t)v;
                else
                    *(int64_t *)d = (int64_t)v;
                break;
            default:
                *(int *)d = (int)v;
                break;
            }
            break;
        }
        case 'd':
        case 'i':
        case 'o':
        case 'u':
        case 'x':
        case 'X':
        case 'b':
        case 'p': {
            /* as strtol (d i), strtoul (o u x b) with the base, or the
               output of %p (0x and hexadecimal digits) */
            int base = conv == 'd' || conv == 'u' ? 10 : conv == 'i' ? 0 : conv == 'o' ? 8 : conv == 'b' ? 2 : 16;
            int neg = 0, digits = 0, over = 0, sgn = conv == 'd' || conv == 'i';
            unsigned __axiom_int64 v = 0;
            long left = width;
#define TAKE() (left < 0 || left-- > 0 ? GET() : (c = -1))
            TAKE();
            if (c == '+' || c == '-') {
                neg = c == '-';
                TAKE();
            }
            if (c == '0') {
                digits = 1;
                TAKE();
                if ((base == 16 || base == 0) && (c == 'x' || c == 'X')) {
                    /* the prefix: a digit must follow */
                    base = 16;
                    digits = 0;
                    TAKE();
                }
                else if ((base == 2 || (base == 0 && in->c23)) && (c == 'b' || c == 'B')) {
                    /* (%b, and %i in C23 mode: the 0b prefix, as strtol's
                       C23 subject sequence has it) */
                    base = 2;
                    digits = 0;
                    TAKE();
                }
                else if (base == 0) {
                    base = 8;
                }
            } else if (base == 0) {
                base = 10;
            }
            for (;; TAKE()) {
                int dv = c >= '0' && c <= '9' ? c - '0'
                       : c >= 'a' && c <= 'z' ? c - 'a' + 10
                       : c >= 'A' && c <= 'Z' ? c - 'A' + 10 : 99;
                if (c < 0 || dv >= base)
                    break;
                digits = 1;
                if (v > (~(unsigned __axiom_int64)0 - (unsigned)dv) / (unsigned)base)
                    over = 1;
                v = v * (unsigned)base + (unsigned)dv;
            }
            UNGET();
#undef TAKE
            if (!digits)
                goto matching_failure;
            convs++;
            if (suppress)
                break;
            /* (out of range: the nearest value, as strtol and strtoul give) */
            if (over)
                v = ~(unsigned __axiom_int64)0;
            if (sgn) {
                if (neg)
                    v = v > (unsigned __axiom_int64)1 << 63 ? (unsigned __axiom_int64)1 << 63 : 0 - v;
                else if (v > ((unsigned __axiom_int64)1 << 63) - 1)
                    v = ((unsigned __axiom_int64)1 << 63) - 1;
            } else if (neg) {
                v = 0 - v;
            }
            if (conv == 'p') {
                *(void **)dst = (void *)(size_t)v;
            } else {
                switch (lm) {
                case 'H':
                    *(unsigned char *)dst = (unsigned char)v;
                    break;
                case 'h':
                    *(unsigned short *)dst = (unsigned short)v;
                    break;
                case 'l':
                    *(unsigned long *)dst = (unsigned long)v;
                    break;
                case 'L':
                case 'j':
                    *(unsigned __axiom_int64 *)dst = v;
                    break;
                case 'z':
                case 't':
                    *(size_t *)dst = (size_t)v;
                    break;
                case 'w':
                    if (bits == 8)
                        *(uint8_t *)dst = (uint8_t)v;
                    else if (bits == 16)
                        *(uint16_t *)dst = (uint16_t)v;
                    else if (bits == 32)
                        *(uint_least32_t *)dst = (uint_least32_t)v;
                    else
                        *(uint64_t *)dst = (uint64_t)v;
                    break;
                default:
                    *(unsigned int *)dst = (unsigned int)v;
                    break;
                }
            }
            assigned++;
            break;
        }
        case 'a':
        case 'A':
        case 'e':
        case 'E':
        case 'f':
        case 'F':
        case 'g':
        case 'G': {
            struct __axc_num num;
            char small[804];
            int range;
            size_t taken, valid, cap = (size_t)__axc_num_digits(lm == 'D');
            num.d = small;
            if (cap > sizeof small) {
                num.d = __axc_sys_alloc(cap);
                if (num.d == NULL)
                    goto matching_failure;
            } else {
                cap = sizeof small;
            }
            num.cap = (int)cap;
            num.nohex = lm >= '1' && lm <= '3'; /* (strtod32's subject sequence: no hexadecimal form) */
            if (!__axc_scan_num(in, width, &num, &taken, &valid)) {
                if (num.d != small)
                    __axc_sys_free(num.d, cap);
                goto input_failure;
            }
            in->count += taken;
            if (taken == 0 || valid != taken) {
                if (num.d != small)
                    __axc_sys_free(num.d, cap);
                goto matching_failure;
            }
            convs++;
            if (!suppress) {
                if (lm >= '1' && lm <= '3')
                    __axc_num_dec(&num, lm - '0', dst, &range);
                else if (lm == 'D')
                    *(long double *)dst = __axc_num_long_double(&num, &range);
                else if (lm == 'l')
                    *(double *)dst = __axc_num_double(&num, &range);
                else
                    *(float *)dst = __axc_num_float(&num, &range);
                assigned++;
            }
            if (num.d != small)
                __axc_sys_free(num.d, cap);
            break;
        }
        case 'c':
        case 's':
        case '[': {
            /* characters: c exactly width (default 1), s up to white
               space, [ those of the scanset; l stores wide characters */
            unsigned char set[32];
            const CH *spec = NULL, *spec_end = NULL;
            int invert = 0, n = 0;
            long left = conv == 'c' && width < 0 ? 1 : width;
            char *out = (char *)dst;
            wchar_t *wout = (wchar_t *)dst;
            mbstate_t st;
            {
                unsigned char *z = (unsigned char *)&st;
                size_t k;
                for (k = 0; k < sizeof st; k++)
                    z[k] = 0;
            }
            if (conv == '[') {
                /* the scanset: ^ inverts; ] first is a member; a-z is a
                   range (implementation-defined, 7.21.6.2p12) */
                int k;
                if (*p == '^') {
                    invert = 1;
                    p++;
                }
                spec = p;
                if (*p == ']')
                    p++;
                while (*p != 0 && *p != ']')
                    p++;
                if (*p == 0)
                    goto matching_failure;
                spec_end = p++;
                for (k = 0; k < 32; k++)
                    set[k] = 0;
                {
                    const CH *s = spec;
                    for (; s < spec_end; s++) {
                        unsigned long lo = (unsigned long)*s, hi = lo, x;
                        if (s[1] == '-' && s + 2 < spec_end && (unsigned long)s[2] >= lo) {
                            hi = (unsigned long)s[2];
                            s += 2;
                        }
                        for (x = lo; x <= hi && x < 256; x++)
                            set[x >> 3] |= (unsigned char)(1u << (x & 7));
                    }
                }
            }
            for (;;) {
                int ok;
                if (left == 0)
                    break;
                if (GET() < 0)
                    break;
                if (conv == 's') {
                    ok = !SF_SPACE(c);
                } else if (conv == '[') {
#if SF_WIDE
                    if ((unsigned long)c < 256) {
                        ok = (set[c >> 3] >> (c & 7)) & 1;
                    } else {
                        const CH *s = spec;
                        ok = 0;
                        for (; s < spec_end; s++) {
                            unsigned long lo = (unsigned long)*s, hi = lo;
                            if (s[1] == '-' && s + 2 < spec_end && (unsigned long)s[2] >= lo) {
                                hi = (unsigned long)s[2];
                                s += 2;
                            }
                            if ((unsigned long)c >= lo && (unsigned long)c <= hi)
                                ok = 1;
                        }
                    }
#else
                    ok = (set[c >> 3] >> (c & 7)) & 1;
#endif
                    ok ^= invert;
                } else {
                    ok = 1;
                }
                if (!ok) {
                    UNGET();
                    break;
                }
                n++;
                if (left > 0)
                    left--;
                if (suppress)
                    continue;
#if SF_WIDE
                if (lm == 'l') {
                    *wout++ = (wchar_t)c;
                } else {
                    /* a wide character to its multibyte bytes */
                    char mb[16];
                    size_t r = wcrtomb(mb, (wchar_t)c, &st), k;
                    if (r == (size_t)-1)
                        goto matching_failure;
                    for (k = 0; k < r; k++)
                        *out++ = mb[k];
                }
#else
                if (lm == 'l') {
                    /* multibyte bytes to wide characters (mbrtowc) */
                    char b = (char)c;
                    wchar_t w;
                    size_t r = mbrtowc(&w, &b, 1, &st);
                    if (r == (size_t)-1)
                        goto matching_failure;
                    if (r != (size_t)-2)
                        *wout++ = w;
                } else {
                    *out++ = (char)c;
                }
#endif
            }
            if (n == 0) {
                if (c < 0)
                    goto input_failure;
                goto matching_failure;
            }
            if (conv == 'c' && left > 0) {
                /* (fewer than the width: the end of the input) */
                goto input_failure;
            }
            convs++;
            if (!suppress) {
                if (conv != 'c') {
                    if (lm == 'l')
                        *wout = 0;
                    else
                        *out = 0;
                }
                assigned++;
            }
            break;
        }
        default:
            /* (not a conversion of the standard's) */
            goto matching_failure;
        }
    }
    return assigned;
input_failure:
    /* 7.21.6.2p16 */
    if (convs == 0 && assigned == 0)
        return EOF;
    return assigned;
matching_failure:
    return assigned;
#undef GET
#undef UNGET
}

#undef SF_SPACE
