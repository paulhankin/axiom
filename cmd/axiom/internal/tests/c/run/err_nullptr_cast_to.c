/* flags: --c-std=c23 */
/* expect-error: C3028 */
/* no type other than nullptr_t converts to nullptr_t by a cast */
#include <stddef.h>
int main(void) { nullptr_t z = (nullptr_t)0; return z == nullptr; }
