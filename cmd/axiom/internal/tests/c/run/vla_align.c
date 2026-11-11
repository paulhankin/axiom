/* Variable length arrays aligned beyond the stack's 16 bytes: by an alignment
   specifier, and by their elements' type; and an array whose size is a
   constant but not an integer constant expression, a VLA. */
#include "host.h"
struct B { _Alignas(32) char c; };
static int f(int n) {
    char pad[3];
    _Alignas(64) int a[n];
    struct B b[n];
    pad[0] = 1;
    a[0] = pad[0];
    b[1].c = 2;
    return (int)((unsigned long)(char *)a % 64) + (int)((unsigned long)(char *)b % 32) + a[0] + b[1].c + (int)sizeof b;
}
static int g(void) {
    const int k = 4;
    int arr[(int)(k + 0.5)];  /* a constant, not an integer constant expression: a VLA */
    return (int)sizeof arr;
}
int main(void) {
    int s = 0;
    for (int i = 1; i < 200; i++) s += f(i % 7 + 2);
    print_i64(s);
    print_i64(g());
    return 0;
}

/* expect: 32341 */
/* expect: 16 */
