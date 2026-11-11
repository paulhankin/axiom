/* flags: --c-std=c23 */
/* expect-error: C3074 */
void g(void);
void f(void) { auto x = g(); }
int main(void) { return 0; }
