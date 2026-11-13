/* the callees of c/run/dec_abi.c, a unit of their own */
#include <stdarg.h>

struct d2 { _Decimal64 a, b; };
struct d3f { _Decimal32 a, b, c; };
struct mix { _Decimal32 a; float b; };
struct dq { _Decimal128 q; };
struct big { _Decimal64 v[5]; int n; };

_Decimal32 add32(_Decimal32 a, _Decimal32 b) { return a + b; }
_Decimal64 add64(_Decimal64 a, _Decimal64 b) { return a + b; }
_Decimal128 add128(_Decimal128 a, _Decimal128 b) { return a + b; }
_Decimal64 many(_Decimal64 a, int i, _Decimal32 b, double d, _Decimal128 c, _Decimal64 e, _Decimal64 f, _Decimal64 g,
                _Decimal64 h, _Decimal64 k, _Decimal64 m, _Decimal32 n) {
    return a + i + b + (_Decimal64)d + (_Decimal64)c + e + f + g + h + k + m + n;
}
struct d2 swap2(struct d2 s) {
    struct d2 r = {s.b, s.a};
    return r;
}
_Decimal32 sum3(struct d3f s) { return s.a + s.b + s.c; }
float mixf(struct mix m) { return (float)m.a + m.b; }
struct dq twiceq(struct dq s) {
    s.q *= 2;
    return s;
}
_Decimal64 bigsum(struct big b) {
    _Decimal64 s = 0;
    for (int i = 0; i < b.n; i++)
        s += b.v[i];
    return s;
}
_Decimal128 vsum(int n, ...) {
    va_list ap;
    _Decimal128 s = 0;
    va_start(ap, n);
    for (int i = 0; i < n; i++) {
        int k = va_arg(ap, int);
        if (k == 32)
            s += va_arg(ap, _Decimal32);
        else if (k == 64)
            s += va_arg(ap, _Decimal64);
        else if (k == 128)
            s += va_arg(ap, _Decimal128);
        else
            s += (_Decimal128)va_arg(ap, double);
    }
    va_end(ap);
    return s;
}
