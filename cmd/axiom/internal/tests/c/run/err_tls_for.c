/* flags: --c-std=c11 */
/* expect-error: C3076 */
void f(void) { for (_Thread_local int i = 0; i < 1; i++) {} }
int main(void) { return 0; }
