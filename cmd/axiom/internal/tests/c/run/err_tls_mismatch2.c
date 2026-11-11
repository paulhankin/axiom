/* flags: --c-std=c11 */
/* expect-error: C3076 */
_Thread_local int y;
int y;
int main(void) { return 0; }
