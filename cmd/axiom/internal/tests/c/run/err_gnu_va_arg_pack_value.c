/* flags: --c-std=gnu17 */
/* expect-error: C3090 */
/* __builtin_va_arg_pack in an inline definition of a function with
   variable arguments, as a value rather than a call's last argument. */
int f(int n, ...);
extern __inline __attribute__((__gnu_inline__)) int g(int n, ...) { int k = __builtin_va_arg_pack(); return f(n, k); }
int main(void) { return g(1, 2); }
