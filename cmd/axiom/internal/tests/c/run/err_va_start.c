/* expect-error: C3900 */
#include <stdarg.h>
int f(int n, ...) { va_list ap; va_start(ap, n); va_end(ap); return n; }
int main(void) { return f(1); }
