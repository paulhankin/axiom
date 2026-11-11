/* expect-error: C2140 */
void f(void) { int *p = (static int[]){ 1 }; (void)p; }
int main(void) { return 0; }
