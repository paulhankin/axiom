/* flags: --c-std=c89 */
/* expect-error: C3002 */
/* A K&R parameter declared twice in the declaration list. */
int f(a) int a; int a; { return a; }
int main(void) { return f(0); }
