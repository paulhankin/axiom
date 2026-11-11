/* 6.9p3: in the operand of sizeof a function with internal linkage need not
   be defined; one defined after its use is. */
#include "host.h"

static int f(void);
static int g(void);

int main(void) {
    print_i64(sizeof(f()) == sizeof(int));
    print_i64(g());
    return 0;
}

static int g(void) { return 5; }

/* expect: 1 */
/* expect: 5 */
