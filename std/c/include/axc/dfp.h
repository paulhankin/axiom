/* axc/dfp.h - the C library's decimal floating functions' private header
   (std/c/lib/math/dfp_*.c, std/c/lib/core/strtod32.c ...): their calls of
   the run-time modules std/c/rt/dfp.ax (__axc_dfn: the exact functions,
   conversions, quantum; __axc_dnan) and std/c/rt/dfp_math.ax (__axc_dm_*:
   the elementary functions), which take an operation's number (the
   generator's order: dfp_*.c), the operands' format (0 _Decimal32, 1
   _Decimal64, 2 _Decimal128) and their encodings (BID) in 16-byte buffers,
   write the result's into r and return an integer result where there is
   one (as 64 bits). The library's units are C17: the types are spelled
   __axiom_decimal32 ..., the compiler's names of _Decimal32 ... in every
   standard. */
#ifndef __AXC_DFP_H
#define __AXC_DFP_H

#include <string.h>

typedef struct {
    _Alignas(16) unsigned __axiom_int64 lo;
    unsigned __axiom_int64 hi;
} __axc_dw;

typedef unsigned __axiom_int64 __axc_dop(unsigned __axiom_int64 op, unsigned __axiom_int64 f, __axc_dw *r,
                                          const __axc_dw *x, const __axc_dw *y, const __axc_dw *z, __axiom_int64 n);
__axc_dop __axc_dfn, __axc_dm_trig, __axc_dm_atrig, __axc_dm_hyp, __axc_dm_exp, __axc_dm_log, __axc_dm_pow,
    __axc_dm_erf, __axc_dm_gamma;
/* nanN(tag): the quiet NaN of format f whose payload tag's digits are */
unsigned __axiom_int64 __axc_dnan(unsigned __axiom_int64 f, __axc_dw *r, const char *tag);

/* an operand into a buffer (zeros after it), a result out of one */
#define __AXC_DFP_IN(w, v) (memset(&(w), 0, sizeof(w)), memcpy(&(w), &(v), sizeof(v)))
#define __AXC_DFP_STORE(p, w) memcpy((p), &(w), sizeof *(p))

static __axiom_decimal32 __axc_out_d32(const __axc_dw *w) {
    __axiom_decimal32 v;
    memcpy(&v, w, sizeof v);
    return v;
}

static __axiom_decimal64 __axc_out_d64(const __axc_dw *w) {
    __axiom_decimal64 v;
    memcpy(&v, w, sizeof v);
    return v;
}

static __axiom_decimal128 __axc_out_d128(const __axc_dw *w) {
    __axiom_decimal128 v;
    memcpy(&v, w, sizeof v);
    return v;
}

#endif
