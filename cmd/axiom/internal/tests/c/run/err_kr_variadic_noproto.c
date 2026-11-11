/* flags: --c-std=c17 */
/* expect-error: C3003 */
int f(int, ...);
int f();
int main(void) { return 0; }
