/* mbsrtowcs (C17 7.29.6.4.1): mbrtowc over a string. Where wchar_t has 16
   bits, a character beyond U+FFFF takes two of the len elements; when only
   one is left, the conversion stops before it. */
#include <axc/text.h>
#include <wchar.h>

/* A whole well-formed UTF-8 character of 2 to 4 bytes at s (table 3-7 of
   the Unicode Standard): its length, its code point in *c; 0 if it is not
   one (then __axc_mb_get reads it, and reports what is wrong). A byte
   after the null character is never read. */
static int whole(const unsigned char *s, unsigned long *c) {
    unsigned int b = s[0], lo = 0x80, hi = 0xbf;
    if (b >= 0xc2 && b <= 0xdf) {
        if ((s[1] & 0xc0) != 0x80)
            return 0;
        *c = ((unsigned long)(b & 0x1f) << 6) | (s[1] & 0x3f);
        return 2;
    }
    if (b == 0xe0 || b == 0xf0)
        lo = b == 0xe0 ? 0xa0 : 0x90;
    if (b == 0xed || b == 0xf4)
        hi = b == 0xed ? 0x9f : 0x8f;
    if (s[1] < lo || s[1] > hi || (s[2] & 0xc0) != 0x80)
        return 0;
    if (b >= 0xe0 && b <= 0xef) {
        *c = ((unsigned long)(b & 0x0f) << 12) | ((unsigned long)(s[1] & 0x3f) << 6) | (s[2] & 0x3f);
        return 3;
    }
    if (b < 0xf0 || b > 0xf4 || (s[3] & 0xc0) != 0x80)
        return 0;
    *c = ((unsigned long)(b & 0x07) << 18) | ((unsigned long)(s[1] & 0x3f) << 12) | ((unsigned long)(s[2] & 0x3f) << 6) |
         (s[3] & 0x3f);
    return 4;
}

size_t mbsrtowcs(wchar_t *restrict dst, const char **restrict src, size_t len, mbstate_t *restrict ps) {
    static _Thread_local mbstate_t internal; /* (each thread's: free of data races) */
    const char *s = *src;
    size_t k = 0;
    int utf8 = __axc_mb_utf8();
    if (!ps)
        ps = &internal;
    if (ps->__axiom_npend) {
        /* (the low surrogate that mbrtowc kept) */
        if (dst) {
            if (len == 0)
                return 0;
            dst[k] = (wchar_t)ps->__axiom_out;
        }
        ps->__axiom_npend = 0;
        k++;
    }
    for (;;) {
        unsigned long c;
        size_t r;
        if (dst && k >= len)
            break;
        if ((unsigned char)*s < 0x80 && ps->__axiom_need == 0) {
            /* (ASCII, in every encoding: the common case first) */
            if (dst)
                dst[k] = (wchar_t)*s;
            if (*s == 0) {
                if (dst)
                    *src = 0;
                return k;
            }
            k++;
            s++;
            continue;
        }
        r = 0;
        if (utf8 && ps->__axiom_need == 0)
            r = (size_t)whole((const unsigned char *)s, &c);
        if (r == 0)
            r = __axc_mb_get(&c, s, (size_t)-1, ps);
        if (r == (size_t)-1) {
            if (dst)
                *src = s;
            return r;
        }
        if (r == 0) {
            if (dst) {
                dst[k] = 0;
                *src = 0;
            }
            return k;
        }
#if __AXIOM_WCHAR_WIDTH__ == 16
        if (c > 0xffff) {
            if (dst) {
                if (len - k < 2) {
                    break;
                }
                dst[k] = (wchar_t)(0xd800 + ((c - 0x10000) >> 10));
                dst[k + 1] = (wchar_t)(0xdc00 + (c & 0x3ff));
            }
            k += 2;
            s += r;
            continue;
        }
#endif
        if (dst)
            dst[k] = (wchar_t)c;
        k++;
        s += r;
    }
    *src = s;
    return k;
}
