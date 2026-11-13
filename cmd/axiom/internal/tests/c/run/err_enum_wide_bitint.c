/* flags: --c-std=gnu23 */
/* expect-error: C3013 */
/* A value of a _BitInt wider than any type an enumeration may have. */
enum E { HUGE = (unsigned _BitInt(200))1 << 150 };
int main(void) { return 0; }
