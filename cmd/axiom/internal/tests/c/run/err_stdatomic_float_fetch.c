/* flags: --c-std=c11 */
/* expect-error: C3093 */
/* 7.17.7.5p1: an atomic integer type */
#include <stdatomic.h>
_Atomic float f;
int main(void) { atomic_fetch_add(&f, 1); return 0; }
