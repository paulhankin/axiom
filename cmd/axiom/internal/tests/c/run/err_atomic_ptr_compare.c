/* flags: --c-std=c11 */
/* expect-error: C3020 */
/* 6.5.9p2: pointers to an atomic type and to its non-atomic version are
   not pointers to compatible types. */
_Atomic int ai;
int i;
int main(void) { return &ai == &i; }
