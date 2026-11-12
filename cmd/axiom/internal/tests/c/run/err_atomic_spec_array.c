/* flags: --c-std=c11 */
/* expect-error: C3092 */
/* 6.7.2.4p3: _Atomic ( type-name ) of an array type. */
_Atomic(int[3]) x;
int main(void) { return 0; }
