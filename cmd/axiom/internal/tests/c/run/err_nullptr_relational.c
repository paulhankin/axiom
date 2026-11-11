/* flags: --c-std=c23 */
/* expect-error: C3020 */
int main(void) { int x = 0; int *p = &x; return p < nullptr; }
