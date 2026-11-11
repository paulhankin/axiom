/* [*] only in the parameters of a prototype. */
/* expect-error: C3085 */
int f(int n) { int a[*]; return 0; }
int main(void) { return f(1); }
