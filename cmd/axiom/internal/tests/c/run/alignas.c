/* flags: --c-std=c11 */
/* Alignment specifiers (C11 6.7.5): _Alignas(constant) and _Alignas(type)
   on objects (file scope, static, automatic, more strictly than the stack
   is aligned) and members (the structure's alignment and size follow), the
   strictest of several, 0 meaning none; _Alignof of such types. */
#include "host.h"
#include <stdalign.h>

struct S { char c; _Alignas(16) int x; char d; };
struct T { char c; _Alignas(double) char d; };
struct W { char c; _Alignas(64) int x; };
_Alignas(64) static char big[3];
_Alignas(32) int g32;
_Alignas(0) short none;
alignas(8) char via_macro;

static int aligned(const void *p, unsigned long a) { return ((unsigned long)p & (a - 1)) == 0; }
static struct W pass(struct W w) { w.x += 1; return w; }

int main(void) {
    _Alignas(128) char loc[5];
    _Alignas(int) _Alignas(16) char two;
    struct S s;
    struct W w = { 1, 2 };
    struct W v = pass(w);
    struct W *p = &(struct W){ 3, 4 };
    int *q = (_Alignas(32) int[]){ 5, 6 };
    print_i64(sizeof(struct S));
    print_i64(_Alignof(struct S));
    print_i64((char *)&s.x - (char *)&s);
    print_i64(sizeof(struct T));
    print_i64(_Alignof(struct T));
    print_i64(sizeof(struct W));
    print_i64(alignof(struct W));
    print_i64(aligned(big, 64) + aligned(&g32, 32) + aligned(&via_macro, 8));
    print_i64(aligned(loc, 128) + aligned(&two, 16) + aligned(&s, 16));
    print_i64(aligned(&w, 64) + aligned(&v, 64) + aligned(p, 64) + aligned(q, 32));
    print_i64(v.x + v.c + p->x + q[1]);
    loc[0] = 1;
    big[0] = 2;
    two = 3;
    print_i64(loc[0] + big[0] + two + none);
    print_i64(__alignas_is_defined + __alignof_is_defined);
    return 0;
}
/* expect: 32 */
/* expect: 16 */
/* expect: 16 */
/* expect: 16 */
/* expect: 8 */
/* expect: 128 */
/* expect: 64 */
/* expect: 3 */
/* expect: 3 */
/* expect: 4 */
/* expect: 14 */
/* expect: 6 */
/* expect: 2 */
