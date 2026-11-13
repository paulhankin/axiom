/* An assembly source on WebAssembly is validated: an error in it is C3097
   at its line. */
/* wasm-only */
/* expect-error: C3097 */
int sub2(int, int);
int main(void) { return sub2(3, 1); }
