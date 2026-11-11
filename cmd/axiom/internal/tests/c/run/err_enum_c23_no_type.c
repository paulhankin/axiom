/* flags: --c-std=c23 */
/* expect-error: C3013 */
/* No integer type holds both -1 and ULLONG_MAX. */
enum E { NEG = -1, HUGE = 0xFFFFFFFFFFFFFFFFull };
int main(void) { return 0; }
