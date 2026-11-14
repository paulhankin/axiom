/* flags: --c-std=gnu11 */
/* expect-warning: C4142 */
/* A GNU dialect takes a conditional expression whose one operand has type
   void and whose other does not (6.5.15p3 asks for both or neither), with a
   warning: the result is void and the other operand's value is discarded.
   Only the operand chosen is evaluated, whether it is the void one or not, a
   statement expression of type void among them. */
#include "host.h"

static int hits;
static int note(int x) { hits += x; return x; }
static void step(void) { hits += 100; }
struct pair { int a, b; };
static struct pair make(int a) { struct pair p = { a, a }; hits += a; return p; }

int main(void) {
    for (int i = 0; i < 2; i++) {
        i ? note(1) : step();
        i ? step() : (void)note(2);
        i ? ({ hits += 1000; (void)0; }) : note(4);
    }
    print_i64(hits);
    hits = 0;
    int k = 1;
    k ? make(7) : (void)0;                     /* a structure value discarded */
    k ? (void)0 : make(9);
    (k ? note(5) : step()), note(6);
    print_i64(hits);
    int lbl = 0;
    lbl ? lbl : ({ for (int j = 0; j < 3; j++) note(10); });
    print_i64(hits);
    return 0;
}
/* expect: 1207 */
/* expect: 18 */
/* expect: 48 */
