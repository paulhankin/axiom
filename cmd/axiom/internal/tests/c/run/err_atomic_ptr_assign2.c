/* flags: --c-std=c11 */
/* expect-error: C3022 */
/* the other way */
int i;
_Atomic int *p = &i;
int main(void) { return 0; }
