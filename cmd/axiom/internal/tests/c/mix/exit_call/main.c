/* A C program that ends by calling exit from deep in its calls: the same
   exit as main's return calls (exit_order), the Axiom module's, which ends
   the process through the Axiom runtime with the status. */
/* uses: ../print.ax ../exit_order/rt.ax */
/* expect-status: 7 */
#include "../print.h"

int at_exit(void (*)(void));
_Noreturn void exit(int);

static void bye(void) { print_cstr("handler"); }

static void deep(int n) {
    if (n == 0)
        exit(7);
    deep(n - 1);
    print_cstr("not reached");
}

int main(void) {
    at_exit(bye);
    deep(5);
    return 1;
    /* expect: handler */
    /* expect: exit */
    /* expect: 7 */
}
