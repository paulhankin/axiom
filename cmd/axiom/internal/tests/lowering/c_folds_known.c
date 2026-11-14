/* What the optimizer decides at compile time, so that the calls of gone()
   go: an arithmetic right shift of -1 (-1 for every count); a comparison
   of two values that a dominating comparison of the same two decides (x < y
   makes x > y, y < x and x == y false, and x != y and x <= y true; x == y
   decides every order); loads from an object defined with a const type
   (6.7.3p6: it cannot change), through the conversion of a floating value
   to an integer; comparisons with a NaN (an ordered one false, != true) and
   beyond an infinity; __builtin_constant_p of a value the optimizer made a
   constant. The calls of kept() stay: an order of the other signedness, a
   const volatile object, a const object defined weak, x >= +inf (x may be
   +inf). 4 calls in all. */
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding --c-std=gnu17 | \bcall\b | 4
// lower: --target=aarch64 -O2 --c-freestanding --c-std=gnu17 | \bbl\b | 4
void gone(void);
void kept(void);
volatile int sink;  /* (stored last: no call is a tail call) */

const double one = 1.0;
static const short tab[3] = { 1, -2, 3 };
const float half = 0.5f;
const volatile int cv = 1;
__attribute__((weak)) const int wk = 1;

void shifts(int x, unsigned n) {
    if ((-1 >> n) != -1)
        gone();
    if ((x < 0) && ((-1 >> n) >= 0))
        gone();
}

void orders(int x, int y) {
    if ((x < y) && (x > y))
        gone();
    if ((x < y) && (y < x))
        gone();
    if ((x <= y) || (y < x)) {
    } else {
        gone();
    }
    if ((x == y) && (x > y))
        gone();
    if ((x > y) && (x != y)) {
    } else if ((x > y)) {
        gone();
    }
    if ((x < y) && ((unsigned)x > (unsigned)y))
        kept();
    sink = 0;
}

void loads(void) {
    if ((int)one != 1)
        gone();
    if (tab[1] != -2)
        gone();
    if ((long)(half * 8) != 4)
        gone();
    if ((unsigned char)(tab[2] * 50.0) != 150)
        gone();
    if (cv != 1)
        kept();
    if (wk != 1)
        kept();
    sink = 0;
}

const double qnan = __builtin_nan("");

void floats(double x) {
    if (qnan == qnan)
        gone();
    if ((x < qnan) || (qnan >= x))
        gone();
    if (qnan != x) {
    } else {
        gone();
    }
    if ((x > __builtin_inf()) || (-__builtin_inf() > x))
        gone();
    if (x >= __builtin_inf())
        kept();
    sink = 0;
}

void constants(void) {
    int k = 4;
    if (!__builtin_constant_p(k))
        gone();
    if (__builtin_constant_p(cv))
        gone();
}
