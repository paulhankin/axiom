/* flags: --c-std=c23 */
/* expect-error: C3073 */
void f(void) { int x = 1; constexpr int a = x; (void)a; }
int main(void) { return 0; }
