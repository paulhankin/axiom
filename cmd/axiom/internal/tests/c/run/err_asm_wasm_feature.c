/* flags: --c-std=gnu17 */
/* wasm-only */
/* expect-error: C3097 */
/* An instruction of an extension that is not enabled (relaxed-simd, tail-call ...). */
typedef int v4 __attribute__((vector_size(16)));
v4 f(v4 a) { v4 r; __asm__("local.get %1 local.get %1 i8x16.relaxed_swizzle local.set %0" : "=r"(r) : "r"(a)); return r; }
int main(void) { return 0; }
