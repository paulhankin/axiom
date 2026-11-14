/* flags: --c-std=gnu17 */
/* An asm operand with '+' (read and written) is one operand: its lvalue is
   evaluated once, for the value read and for the place written (a call in
   it is made once, an increment in it done once), in a register or in
   memory. */
#include "host.h"

static int count;
static int cell[4] = { 10, 20, 30, 40 };

static int *at(int i) {
    count++;
    return &cell[i];
}

int main(void) {
    int i = 1;
    asm("" : "+r"(*at(0)));
    asm("" : "+r"(cell[i++]));
    asm("" : "+m"(*at(2)));
    print_i64(count * 1000 + i * 100 + cell[0] + cell[1] + cell[2]);
    return 0;
}
/* expect: 2260 */
