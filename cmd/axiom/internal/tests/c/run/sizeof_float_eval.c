/* flags: --c-std=c11 */
/* sizeof and _Alignof (6.5.3.4): the operand of sizeof is not evaluated
   (unless a variable length array); string literals, arrays, function
   results, promoted and unpromoted operands; float arithmetic is done in
   float (FLT_EVAL_METHOD 0: no excess precision). */
#include <float.h>
#include "host.h"

static int calls;
static int f(void) { return ++calls; }
static char g(void) { return 'g'; }

int main(void) {
    char c = 0;
    int a[7];
    int x = 1;
    print_i64(sizeof x++);
    print_i64(x);
    print_i64(sizeof f());
    print_i64(calls);
    print_i64(sizeof g());
    print_i64(sizeof +c);
    print_i64(sizeof c);
    print_i64(sizeof (c + c));
    print_i64(sizeof "hello");
    print_i64(sizeof a / sizeof a[0]);
    print_i64(sizeof &a == sizeof(int *));
    print_i64(sizeof *&a / sizeof(int));
    print_i64(sizeof (0, a) == sizeof(int *));
    print_i64(sizeof(char[3][5]));
    print_i64(sizeof 1.0f);
    print_i64(sizeof (1.0f + 1.0f));
    print_i64(sizeof (1.0f + 1.0));
    print_i64(_Alignof(char) + _Alignof(int[4]) * 10);
    print_i64(_Alignof(struct { char a; double b; }) == _Alignof(double));
    print_i64(FLT_EVAL_METHOD);
    volatile float big = 16777216.0f;
    volatile float one = 1.0f;
    float r = big + one - big;
    print_f64(r);
    print_i64(big + one == big);
    double d = big + one;
    print_f64(d - 16777216.0);
    print_f64((double)big + one - big);
    float third = 1.0f / 3.0f;
    print_i64(third == (float)(1.0 / 3.0));
    print_i64(1.0f / 3.0f == 1.0 / 3.0);
    return 0;
}
/* expect: 4 */
/* expect: 1 */
/* expect: 4 */
/* expect: 0 */
/* expect: 1 */
/* expect: 4 */
/* expect: 1 */
/* expect: 4 */
/* expect: 6 */
/* expect: 7 */
/* expect: 1 */
/* expect: 7 */
/* expect: 1 */
/* expect: 15 */
/* expect: 4 */
/* expect: 4 */
/* expect: 8 */
/* expect: 41 */
/* expect: 1 */
/* expect: 0 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 1 */
/* expect: 0 */
