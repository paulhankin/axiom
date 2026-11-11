/* flags: --c-std=c11 */
/* expect-error: C3076 */
void f(void) { _Thread_local int b; }
int main(void) { return 0; }
