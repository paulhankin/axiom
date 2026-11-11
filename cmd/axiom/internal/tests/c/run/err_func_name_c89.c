/* flags: --c-std=c89 */
/* expect-error: C3001 */
/* __func__ is C99's: in C89 it is an undeclared (reserved) identifier. */
int main(void) { return sizeof(__func__); }
