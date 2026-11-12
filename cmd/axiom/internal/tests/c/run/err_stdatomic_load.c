/* flags: --c-std=c11 */
/* expect-error: C3093 */
/* 7.17.7.2: the object is atomic */
#include <stdatomic.h>
int i;
int main(void) { return atomic_load(&i); }
