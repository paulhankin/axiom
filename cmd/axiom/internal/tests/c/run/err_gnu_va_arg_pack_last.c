/* flags: --c-std=gnu17 */
/* expect-error: C3090 */
/* __builtin_va_arg_pack in an inline definition of a function with
   variable arguments, but not as the last of a call's variable arguments
   (here a fixed one's): nothing could replace it. */
int f(int n, ...);
extern __inline __attribute__((__gnu_inline__)) int g(int n, ...) { return f(__builtin_va_arg_pack(), n); }
int main(void) { return g(1, 2); }
