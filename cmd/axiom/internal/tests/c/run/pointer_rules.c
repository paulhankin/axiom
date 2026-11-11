/* flags: --c-std=c11 */
/* Pointers in assignment, ?: and comparisons (6.5.15, 6.5.16.1, 6.5.8,
   6.5.9): the qualifiers of the pointed-to types combine in ?:, a void *
   operand makes the result a pointer to (qualified) void, a null pointer
   constant takes the other operand's type, composite types complete an
   array; qualifiers may be added one level down (char ** to char *const *);
   null pointer constants: integer constant expressions with value 0 and
   their casts to void *; pointer differences and comparisons. */
#include <stddef.h>
#include "host.h"

#define T(x) _Generic((x), int *: 1, const int *: 2, volatile int *: 3, const volatile int *: 4, void *: 5, \
    const void *: 6, int (*)[3]: 7, char *const *: 9, const volatile void *: 10, default: 0)

static int f(void) { return 1; }

int main(void) {
    int a[3] = { 1, 2, 3 };
    int x = 0;
    int *p = &x;
    const int *cp = &x;
    volatile int *vp = &x;
    void *v = &x;
    const void *cv = &x;
    int (*pa)[3] = &a;
    int (*pu)[] = &a;
    volatile int one = 1;
    print_i64(T(one ? p : cp));
    print_i64(T(one ? cp : vp));
    print_i64(T(one ? p : v));
    print_i64(T(one ? cp : v));
    print_i64(T(one ? vp : cv));
    print_i64(T(one ? p : 0));
    print_i64(T(one ? (void *)0 : cp));
    print_i64(T(one ? pa : pu));
    print_i64(sizeof *(one ? pu : pa) / sizeof(int));
    char *s = "s";
    char **pp = &s;
    char *const *cpp = pp;
    print_i64(T(cpp));
    print_i64(**cpp);
    const void *q = pp;
    print_i64(q == (void *)pp);
    int *n1 = 0;
    int *n2 = (void *)0;
    int *n3 = (1 - 1);
    int *n4 = '\0';
    int *n5 = (int)0.0;
    int *n6 = sizeof(int) - sizeof(int);
    print_i64(!n1 + !n2 + !n3 + !n4 + !n5 + !n6);
    print_i64(p == v);
    print_i64(cp == p);
    print_i64(v == cv);
    print_i64(&a[2] - &a[0]);
    print_i64(&a[0] < &a[2]);
    print_i64(&a[2] >= a + 2);
    print_i64(a + 3 > &a[1]);
    print_i64(NULL == p);
    int (*fp)(void) = f;
    print_i64(fp == f);
    print_i64(fp != 0);
    print_i64(fp == (void *)0);
    print_i64((one ? fp : 0)());
    print_i64((one ? f : (void *)0) == f);
    ptrdiff_t d = (char *)&a[1] - (char *)&a[0];
    print_i64(d == sizeof(int));
    return 0;
}
/* expect: 2 */
/* expect: 4 */
/* expect: 5 */
/* expect: 6 */
/* expect: 10 */
/* expect: 1 */
/* expect: 2 */
/* expect: 7 */
/* expect: 3 */
/* expect: 9 */
/* expect: 115 */
/* expect: 1 */
/* expect: 6 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 2 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
