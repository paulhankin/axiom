/* flags: --c-std=c11 */
/* expect-error: C3093 */
/* 7.17.7.1: the object of a store is not const */
#include <stdatomic.h>
const atomic_int c;
int main(void) { atomic_store(&c, 1); return 0; }
