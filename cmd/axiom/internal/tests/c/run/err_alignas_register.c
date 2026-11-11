/* expect-error: C3071 */
void f(void) { register _Alignas(8) int x = 0; (void)x; }
int main(void) { return 0; }
