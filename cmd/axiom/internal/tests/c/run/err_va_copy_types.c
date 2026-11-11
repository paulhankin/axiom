/* expect-error: C3080 */
/* va_copy of something that is not a va_list. */
/* flags: --c-std=c99 */
#include <stdarg.h>
int f(int n, ...) { va_list ap; int x = 0; va_copy(ap, x); return n; }
int main(void) { return f(1); }
