/* flags: --c-std=c11 */
/* expect-error: C3092 */
/* 6.7.3p3: the type an _Atomic qualifier modifies is no array type. */
typedef int A[2];
_Atomic A x;
int main(void) { return 0; }
