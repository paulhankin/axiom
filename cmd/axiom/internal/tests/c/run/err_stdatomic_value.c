/* flags: --c-std=c11 */
/* expect-error: C3022 */
/* the value is converted as by assignment */
#include <stdatomic.h>
int *_Atomic p;
int main(void) { atomic_store(&p, 1.0); return 0; }
