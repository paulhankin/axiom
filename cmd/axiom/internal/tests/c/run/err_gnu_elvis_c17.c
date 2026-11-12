/* flags: --c-std=c17 */
/* expect-error: C2001 */
int main(void) { int a = 1; return a ?: 2; }
