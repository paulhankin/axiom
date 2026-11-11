/* flags: --c-std=c89 */
/* expect-error: C3006 */
/* C89: a qualifier repeated through a typedef name of an array type, whose
   elements have it. */
typedef const int A[2];
const A x = { 1, 2 };
int main(void) { return x[0]; }
