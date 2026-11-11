/* flags: --c-std=c23 */
/* expect-error: C3077 */
[[nodiscard("a", "b")]] int f(void);
int main(void) { return 0; }
