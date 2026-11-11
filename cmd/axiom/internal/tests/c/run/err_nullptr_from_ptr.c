/* flags: --c-std=c23 */
/* expect-error: C3022 */
/* a nullptr_t object takes a null pointer constant or nullptr_t, not a pointer */
#include <stddef.h>
int main(void) { void *v = 0; nullptr_t z = v; return z == nullptr; }
