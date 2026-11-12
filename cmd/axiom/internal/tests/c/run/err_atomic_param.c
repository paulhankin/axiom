/* flags: --c-std=c11 */
/* expect-error: C3003 */
/* 6.7.6.3p15: a parameter of atomic type keeps it in the function type. */
void f(_Atomic int);
void f(int);
int main(void) { return 0; }
