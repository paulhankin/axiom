/* expect-error: C2001 */
int f(void) { __builtin_va_list ap; return __builtin_va_arg(ap, 1); }
