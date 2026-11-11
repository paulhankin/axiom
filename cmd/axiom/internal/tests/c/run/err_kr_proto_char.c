/* flags: --c-std=c89 */
/* expect-error: C3003 */
/* the prototype's parameter must be compatible with the promoted type (int) */
int f(char);
int f(c) char c; { return c; }
int main(void) { return f(1); }
