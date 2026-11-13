/* flags: --c-std=gnu23 */
/* expect-error: C3013 */
/* The constant after the largest unsigned __int128 has no type. */
enum E { TOP = ~(unsigned __int128)0, PAST };
int main(void) { return 0; }
