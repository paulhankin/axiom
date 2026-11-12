/* flags: --c-std=c11 */
/* expect-error: C3092 */
/* 6.7.2.4p3: _Atomic ( type-name ) of a function type. */
_Atomic(int (void)) *p;
int main(void) { return 0; }
