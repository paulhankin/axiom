/* flags: --c-std=c99 */
/* expect-error: C2001 */
int a[4] = { [0 ... 2] = 1 };
int main(void) { return 0; }
