/* flags: --c-std=c11 */
/* expect-error: C3092 */
/* 6.7.2.4p3: _Atomic ( type-name ) of a qualified type. */
_Atomic(const int) x;
int main(void) { return 0; }
