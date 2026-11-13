/* flags: --c-std=gnu17 */
/* expect-error: C3013 */
/* Before C23 an enumeration constant is an int. */
enum E { WIDE = (__int128)1 << 64 };
int main(void) { return 0; }
