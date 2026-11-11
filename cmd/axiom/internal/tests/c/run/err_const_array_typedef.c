/* expect-error: C3021 */
/* 6.7.3p8: the qualifiers of an array type (through a typedef name) are its
   element type's: the elements of a const array are const. */
typedef int A[2];
const A a = { 1, 2 };
int main(void) { a[0] = 3; return 0; }
