/* No computed goto (an extension). */
/* expect-error: C2001 */
int f(int x) { goto *x; }
int main(void) { return f(0); }
