/* flags: --c-std=c11 */
/* expect-error: C3093 */
/* 7.17.7.5: only fetch_add and fetch_sub on an atomic pointer */
#include <stdatomic.h>
int *_Atomic p;
int main(void) { atomic_fetch_or(&p, 1); return 0; }
