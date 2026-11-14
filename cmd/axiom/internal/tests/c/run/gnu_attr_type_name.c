/* flags: --c-std=gnu11 */
/* expect-warning: C4052 */
/* A GNU dialect takes __attribute__((...)) in a type name: at its start
   (a cast, sizeof, a compound literal, _Generic's associations), after
   its specifiers, inside an abstract declarator before '*' (after the '('
   of a parenthesized one), among a pointer's qualifiers and in a function
   declarator's parameters. They appertain to the type; one that cannot
   (noinline is a function's) is ignored with a warning. */
#include "host.h"

#define NI __attribute__((__noinline__))
#define UN __attribute__((unused))

static int seven(void) { return 7; }
static int add(int a, int b) { return a + b; }

int main(void) {
    void *code = (void *)seven;
    int a = ((NI int (*)(void))code)();
    int b = ((int (NI *)(void))code)();
    int c = ((int (* NI)(void))code)();
    int d = ((int (*)(UN int, int UN))(void *)add)(2, 3);
    int e = (int)sizeof(UN long long);
    int f = (UN struct { int x; }){ 9 }.x;
    int g = _Generic(a, UN int: 1, default: 0);
    int h = (int UN)2;
    int (*p)(void) = (NI int (*)(void))0;
    print_i64(a + b + c);
    print_i64(d);
    print_i64(e);
    print_i64(f + g + h);
    print_i64(p == 0);
    return 0;
}
/* expect: 21 */
/* expect: 5 */
/* expect: 8 */
/* expect: 12 */
/* expect: 1 */
