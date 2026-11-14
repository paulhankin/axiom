/* flags: --c-std=gnu11 */
/* expect-error: C3090 */
/* The resolver of an indirect function is a function the unit defines (and
   on a target without indirect functions, WebAssembly, the attribute is an
   error itself). */
int f(void) __attribute__((ifunc("nowhere")));
int main(void) { return f(); }
