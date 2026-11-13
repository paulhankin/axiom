/* flags: --c-std=gnu23 */
/* expect-error: C3013 */
/* No integer type holds both -1 and the largest unsigned __int128. */
enum E { NEG = -1, TOP = ~(unsigned __int128)0 };
int main(void) { return 0; }
