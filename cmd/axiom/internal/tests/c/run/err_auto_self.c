/* flags: --c-std=c23 */
/* expect-error: C3001 */
void f(void) { auto x = x + 1; }
int main(void) { return 0; }
