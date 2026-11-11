/* An object-like macro redefined as function-like. */
/* expect-error: C1060 */
#define M 1
#define M() 1
