/* flags: --c-std=c23 */
/* expect-error: C3074 */
void f(void) { auto a = 1, b = 2; }
int main(void) { return 0; }
