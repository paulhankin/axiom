/* flags: --c-std=c99 */
/* expect-error: C3006 */
/* 6.7.4p1: function specifiers only in the declaration of a function. */
inline typedef int T;
int main(void) { return 0; }
