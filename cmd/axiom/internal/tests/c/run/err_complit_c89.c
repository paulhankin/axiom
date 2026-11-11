/* flags: --c-std=c89 */
/* expect-error: C2109 */
int *p;
void f(void) { p = (int[]){ 1 }; }
int main(void) { return 0; }
