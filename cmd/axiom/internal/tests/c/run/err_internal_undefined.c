/* expect-error: C3045 */
/* 6.9p3: a function with internal linkage that is used in an expression
   (not only in the operand of sizeof) must be defined in the unit. */
static int f(void);
int main(void) { return f(); }
