/* flags: --c-std=c11 */
/* expect-error: C3020 */
/* 6.5.15p3 */
_Atomic int ai;
int i;
int main(void) { return *(1 ? &ai : &i); }
