/* flags: --c-std=c23 */
/* nullptr through a variadic function (C23 7.16.1.1: va_arg of nullptr_t,
   and of a pointer type when a nullptr_t was passed, which has its
   representation), on every target. */
#include "host.h"
#include <stdarg.h>
#include <stddef.h>
static int f(int n, ...) {
  va_list ap; va_start(ap); int r = 0;
  for (int i = 0; i < n; i++) { nullptr_t z = va_arg(ap, nullptr_t); r += (z == nullptr); }
  void *p = va_arg(ap, void *); r += (p == nullptr) * 10;
  char *q = va_arg(ap, char *); r += (q == 0) * 100;
  int last = va_arg(ap, int);
  va_end(ap); return r + last * 1000;
}
int main(void) { print_i64(f(2, nullptr, nullptr, nullptr, nullptr, 7)); return 0; }
/* expect: 7112 */
