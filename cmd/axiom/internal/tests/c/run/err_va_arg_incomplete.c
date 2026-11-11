/* expect-error: C3080 */
/* va_arg of an incomplete type. */
#include <stdarg.h>
struct t;
int f(int n, ...) { va_list ap; va_start(ap, n); va_arg(ap, struct t); va_end(ap); return n; }
int main(void) { return f(1); }
