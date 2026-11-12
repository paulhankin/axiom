/* flags: --c-std=c11 */
/* expect-error: C3023 */
/* the builtins check their arguments' number */
#include <stdatomic.h>
atomic_int a;
int main(void) { return __builtin_atomic_load(&a); }
