/* flags: --c-std=c89 */
/* expect-error: C2108 */
int a[3] = { [1] = 1 };
int main(void) { return 0; }
