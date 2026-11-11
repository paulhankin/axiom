/* flags: --c-std=c23 */
/* expect-error: C3023 */
/* () is (void) in C23 */
int f();
int main(void) { return f(1); }
