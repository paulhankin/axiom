/* flags: --c-std=c23 */
/* expect-error: C3074 */
void f(void) { auto x = { 1 }; }
int main(void) { return 0; }
