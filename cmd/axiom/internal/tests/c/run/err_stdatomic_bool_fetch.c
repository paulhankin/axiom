/* flags: --c-std=c11 */
/* expect-error: C3093 */
/* 7.17.7.5p1: not on atomic_bool */
#include <stdatomic.h>
atomic_bool b;
int main(void) { return atomic_fetch_add(&b, 1); }
