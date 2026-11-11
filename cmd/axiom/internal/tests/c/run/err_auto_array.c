/* flags: --c-std=c23 */
/* expect-error: C3074 */
void f(void) { auto a[2] = { 1, 2 }; }
int main(void) { return 0; }
