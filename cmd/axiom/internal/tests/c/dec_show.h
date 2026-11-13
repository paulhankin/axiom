/* dec_show.h - the decimal floating tests' output (c/run/dec_*.c,
   c/lib/math/dec_*.c): a value of _Decimal32, _Decimal64 or _Decimal128
   written as IEEE 754-2019's to-scientific-string writes it (5.12.2: its
   coefficient's digits and its quantum, plain when the exponent is at most
   0 and the adjusted exponent at least -6, else d.ddd followed by E and
   the exponent; Infinity, NaN, sNaN with a payload), from its encoding
   (BID), with print_str. */
#ifndef DEC_SHOW_H
#define DEC_SHOW_H

#include <stddef.h>
#include <string.h>

void print_str(const char *p, size_t n);

static void dec_show_bits(unsigned long long lo, unsigned long long hi, int bytes) {
    char out[80], dg[40];
    int n = 0, nd = 0, kb = bytes * 8, eb = bytes == 4 ? 8 : bytes == 8 ? 10 : 14, cb = kb - 1 - eb, i, q, adj;
    int bias = bytes == 4 ? 101 : bytes == 8 ? 398 : 6176, p = bytes == 4 ? 7 : bytes == 8 ? 16 : 34;
    unsigned long long top = bytes == 16 ? hi : lo, g, e, c0, c1 = 0;
    int neg = (int)((top >> (bytes == 16 ? 63 : kb - 1)) & 1), nan = 0, snan = 0;
    g = (top >> (bytes == 16 ? 58 : kb - 6)) & 31;
    if (neg)
        out[n++] = '-';
    if (g == 30) {
        const char *s = "Infinity";
        while (*s)
            out[n++] = *s++;
        print_str(out, (size_t)n);
        return;
    }
    if (g == 31) {
        nan = 1;
        snan = (int)((top >> (bytes == 16 ? 57 : kb - 7)) & 1);
        c0 = bytes == 16 ? lo : lo & ((1ULL << (cb - 3)) - 1);
        c1 = bytes == 16 ? hi & ((1ULL << 46) - 1) : 0;
        q = 0;
    } else if ((g >> 3) == 3) {
        if (bytes == 16) {
            c0 = c1 = 0;
            e = (hi >> 47) & 0x3fff;
        } else {
            e = (lo >> (cb - 2)) & ((1ULL << eb) - 1);
            c0 = (4ULL << (cb - 2)) | (lo & ((1ULL << (cb - 2)) - 1));
        }
        q = (int)e - bias;
    } else if (bytes == 16) {
        e = (hi >> 49) & 0x3fff;
        c1 = hi & 0x1ffffffffffffULL;
        c0 = lo;
        q = (int)e - bias;
    } else {
        e = (lo >> cb) & ((1ULL << eb) - 1);
        c0 = lo & ((1ULL << cb) - 1);
        q = (int)e - bias;
    }
    /* the coefficient's digits, least significant first (128 bits / 10) */
    do {
        unsigned long long r = 0, w[4] = {c1 >> 32, c1 & 0xffffffffULL, c0 >> 32, c0 & 0xffffffffULL};
        for (i = 0; i < 4; i++) {
            unsigned long long cur = (r << 32) | w[i];
            w[i] = cur / 10;
            r = cur % 10;
        }
        c1 = (w[0] << 32) | w[1];
        c0 = (w[2] << 32) | w[3];
        dg[nd++] = (char)('0' + r);
    } while ((c0 | c1) != 0 && nd < 39);
    if (nd > p || (nan && nd > p - 1)) {
        nd = 1; /* (non-canonical: 0) */
        dg[0] = '0';
    }
    if (nan) {
        const char *s = snan ? "sNaN" : "NaN";
        while (*s)
            out[n++] = *s++;
        if (!(nd == 1 && dg[0] == '0'))
            for (i = nd - 1; i >= 0; i--)
                out[n++] = dg[i];
        print_str(out, (size_t)n);
        return;
    }
    adj = q + nd - 1;
    if (q <= 0 && adj >= -6) {
        if (nd + q <= 0) {
            out[n++] = '0';
            out[n++] = '.';
            for (i = 0; i < -(nd + q); i++)
                out[n++] = '0';
            for (i = nd - 1; i >= 0; i--)
                out[n++] = dg[i];
        } else {
            for (i = nd - 1; i >= 0; i--) {
                if (i == -q - 1 && q < 0)
                    out[n++] = '.';
                out[n++] = dg[i];
            }
        }
    } else {
        char t[8];
        int k = 0, x = adj < 0 ? -adj : adj;
        out[n++] = dg[nd - 1];
        if (nd > 1)
            out[n++] = '.';
        for (i = nd - 2; i >= 0; i--)
            out[n++] = dg[i];
        out[n++] = 'E';
        out[n++] = adj < 0 ? '-' : '+';
        do {
            t[k++] = (char)('0' + x % 10);
            x /= 10;
        } while (x != 0);
        while (k > 0)
            out[n++] = t[--k];
    }
    print_str(out, (size_t)n);
}

#define DEC_BYTES(x) ((int)sizeof(x))
#define dec_show(x)                                                       \
    do {                                                                  \
        unsigned long long dec_w_[2] = {0, 0};                            \
        typeof(x) dec_v_ = (x);                                       \
        memcpy(dec_w_, &dec_v_, sizeof dec_v_);                 \
        dec_show_bits(dec_w_[0], dec_w_[1], DEC_BYTES(dec_v_));           \
    } while (0)

#endif
