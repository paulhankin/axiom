/* expect-error: C3023 */
/* va_start with one argument before C23. */
/* flags: --c-std=c17 */
int f(int n, ...) { __builtin_va_list ap; __builtin_va_start(ap); return n; }
int main(void) { return f(1); }
