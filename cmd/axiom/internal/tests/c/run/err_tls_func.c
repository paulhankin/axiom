/* flags: --c-std=c11 */
/* expect-error: C3076 */
_Thread_local void g(void);
int main(void) { return 0; }
