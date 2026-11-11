/* flags: --c-std=c23 */
/* expect-error: C3013 */
/* The constant after ULLONG_MAX has no type. */
enum E { HUGE = 0xFFFFFFFFFFFFFFFFull, PAST };
int main(void) { return 0; }
