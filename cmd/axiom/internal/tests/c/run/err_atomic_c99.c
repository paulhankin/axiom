/* flags: --c-std=c99 */
/* expect-error: C2100 */
/* _Atomic is a C11 keyword. */
_Atomic int x;
int main(void) { return 0; }
