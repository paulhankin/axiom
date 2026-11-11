/* flags: --c-std=c23 */
/* expect-error: C3222 */
void f(void) { int *p = &(register int){ 1 }; (void)p; }
int main(void) { return 0; }
