/* flags: --c-std=gnu17 */
/* wasm-only */
/* expect-error: C3097 */
/* An input tied to an output shares its local: they must be of one value type. */
int f(long long x) { int r; __asm__("" : "=r"(r) : "0"(x)); return r; }
int main(void) { return f(1); }
