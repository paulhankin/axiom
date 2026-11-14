/* flags: --c-std=gnu17 */
/* Label addresses (a GNU dialect) stored by the code at run time, in a
   local array's initializer and by assignments, are distinct values, and
   `goto *` reaches each one's label; a label may have the name of a
   function (labels have a name space of their own, 6.2.3). The addresses of
   elements past the first of a const table, read from another const table
   (which the optimizer folds), are those elements' addresses too. */
#include "host.h"

static int pick(int i) {
    void *j[] = { &&pick, &&b, &&c };
    goto *j[i];
pick:
    return 2;
b:
    return 3;
c:
    return 5;
}

static int pick2(int i) {
    void *j[3];
    j[2] = &&c;
    j[0] = &&a;
    j[1] = &&b;
    if (j[0] == j[1] || j[1] == j[2])
        return -1;
    goto *j[i];
a:
    return 20;
b:
    return 30;
c:
    return 50;
}

static const int vals[4] = { 10, 20, 30, 40 };
static const int *const ptrs[3] = { &vals[1], &vals[3], vals };

int main(void) {
    print_i64(pick(0) * 100 + pick(1) * 10 + pick(2));
    print_i64(pick2(0) + pick2(1) + pick2(2));
    print_i64(*ptrs[0] * 100 + *ptrs[1] + *ptrs[2] + ptrs[1][-1]);
    return 0;
}
/* expect: 235 */
/* expect: 100 */
/* expect: 2080 */
