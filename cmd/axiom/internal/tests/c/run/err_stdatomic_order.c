/* flags: --c-std=c11 */
/* expect-error: C3093 */
/* 7.17.3: a memory order */
#include <stdatomic.h>
atomic_int a;
int main(void) { return atomic_load_explicit(&a, 1.5); }
