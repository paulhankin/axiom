/* flags: --c-std=gnu17 */
/* expect-error: C3090 */
/* __builtin_va_arg_pack outside the inline definition of a function with
   variable arguments. */
int f(int n, ...);
int g(int n, ...) { return f(n, __builtin_va_arg_pack()); }
int main(void) { return 0; }
