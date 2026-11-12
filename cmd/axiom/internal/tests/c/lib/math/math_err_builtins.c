/* <math.h>'s and <tgmath.h>'s builtins diagnose what they do not take. */
/* flags: --c-std=c11 */
/* expect-error: C3082 */
#include <tgmath.h>

struct s { int a; };

int main(void)
{
    struct s v = { 1 };
    double complex z = 1.0;
    int r = isnan(v);
    double m = fmod(z, 2.0); /* (no complex fmod) */
    return r + (int)m;
}
