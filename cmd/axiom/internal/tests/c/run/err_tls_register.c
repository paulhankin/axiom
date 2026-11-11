/* flags: --c-std=c11 */
/* expect-error: C3076 */
void f(void) { register _Thread_local int r; }
int main(void) { return 0; }
