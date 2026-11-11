/* flags: --c-std=c23 */
/* expect-error: C3021 */
constexpr int a = 1;
void f(void) { a = 2; }
int main(void) { return 0; }
