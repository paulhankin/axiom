/* expect-error: C3031 */
void f(void) { static int *q = &(int){ 2 }; (void)q; }
int main(void) { return 0; }
