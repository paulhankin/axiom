/* flags: --c-std=c99 */
/* expect-error: C3003 */
int f(c) char c; { return c; }
int f(int, int);
int main(void) { return 0; }
