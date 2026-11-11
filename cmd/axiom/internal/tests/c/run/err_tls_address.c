/* flags: --c-std=c11 */
/* expect-error: C3031 */
_Thread_local int z;
static int *p = &z;
int main(void) { return 0; }
