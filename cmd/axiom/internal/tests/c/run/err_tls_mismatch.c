/* flags: --c-std=c11 */
/* expect-error: C3076 */
int x;
_Thread_local int x;
int main(void) { return 0; }
