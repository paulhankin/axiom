/* flags: --c-std=c99 */
/* expect-error: C2100 */
_Thread_local int x;
int main(void) { return 0; }
