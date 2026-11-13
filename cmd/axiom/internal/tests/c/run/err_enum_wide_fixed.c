/* flags: --c-std=gnu23 */
/* expect-error: C3013 */
/* The value is outside the fixed underlying type __int128. */
enum E : __int128 { TOP = ~(unsigned __int128)0 };
int main(void) { return 0; }
