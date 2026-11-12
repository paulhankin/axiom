/* flags: --c-std=c11 */
/* expect-error: C3003 */
/* the result type is compared with its _Atomic */
_Atomic int f(void);
int f(void);
int main(void) { return 0; }
