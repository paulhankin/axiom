/* expect-error: C3080 */
/* va_arg of an array type. */
#include <stdarg.h>
int f(int n, ...) { va_list ap; va_start(ap, n); va_arg(ap, int[2]); va_end(ap); return n; }
int main(void) { return f(1); }
