/* flags: --c-std=c23 */
/* expect-error: C3035 */
int n = 8;
int main(void) { _BitInt(n) x = 0; return x; }
