/* flags: --c-std=c11 */
/* expect-error: C3092 */
/* 6.7.2.4p3: _Atomic ( type-name ) of an atomic type. */
typedef _Atomic int AI;
_Atomic(AI) x;
int main(void) { return 0; }
