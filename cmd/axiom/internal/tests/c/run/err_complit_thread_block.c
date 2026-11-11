/* flags: --c-std=c23 */
/* expect-error: C3076 */
void f(void) { int *p = &(thread_local int){ 1 }; (void)p; }
int main(void) { return 0; }
