/* expect-error: C3080 */
/* va_arg of something that is not a va_list. */
#include <stdarg.h>
int f(int n, ...) { int ap = 0; return va_arg(ap, int) + n; }
int main(void) { return f(1); }
