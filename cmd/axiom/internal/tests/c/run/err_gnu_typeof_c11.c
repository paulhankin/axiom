/* flags: --c-std=c11 */
/* expect-error: C2100 */
int main(void) { int a = 1; typeof(a) b = a; return b; }
