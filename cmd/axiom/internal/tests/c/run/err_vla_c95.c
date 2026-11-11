/* A variable length array is a C99 feature (C95 has none either). */
/* flags: --c-std=c95 */
/* expect-error: C3080 */
int f(int n) { return sizeof(int[n]); }
int main(void) { return f(1); }
