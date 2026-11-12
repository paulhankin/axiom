/* flags: --c-std=c11 */
/* expect-error: C3093 */
/* 7.17.7.5p3: a pointer to an object type */
#include <stdatomic.h>
void *_Atomic p;
int main(void) { atomic_fetch_add(&p, 1); return 0; }
