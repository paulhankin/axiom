/* flags: --c-std=c11 */
/* expect-error: C3092 */
/* 6.7.3p3: nor a function type. */
typedef int F(void);
_Atomic F *p;
int main(void) { return 0; }
