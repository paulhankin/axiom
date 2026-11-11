/* flags: --c-std=c89 */
/* Definitions with identifier lists (6.9.1): parameters declared char,
   short and float receive their promoted arguments and convert them;
   calls through declarations without prototypes promote the arguments
   (6.5.2.2p6); a prototype compatible with such a definition (6.7.6.3p15)
   converts them to its own (promoted) types; a parameter without a
   declaration is int (C89). An argument must have its parameter's promoted
   type (4L for a long), or the call is undefined. */
#include "host.h"

int sum();
double half();
int proto_first(int, double);

int sum(c, s, f, d)
    char c;
    short s;
    float f;
    double d;
{
    return c + s + (int)(f * 2) + (int)d;
}

double half(f) float f; { return f / 2; }

int proto_first(c, f) unsigned char c; float f; { return c + (int)f; }

int implicit(a, b) long b; { return a * 10 + (int)b; }

static int narrow(c) signed char c; { return c; }

static int through(int (*fp)()) { return fp(3, 4L); }

int main() {
    char c = 1;
    float f = 2.5f;
    print_i64(sum(c, (short)20, f, 300.0));
    print_i64(sum(1, 2, 3.0, 4.0));
    print_f64(half(3.0f));
    print_f64(half(5.0));
    print_i64(proto_first(300, 7.9));
    print_i64(implicit(4, 5L));
    print_i64(narrow(127 + 1));
    print_i64(through(implicit));
    return 0;
}

/* expect: 326 */
/* expect: 13 */
/* expect: 1.5 */
/* expect: 2.5 */
/* expect: 51 */
/* expect: 45 */
/* expect: -128 */
/* expect: 34 */
