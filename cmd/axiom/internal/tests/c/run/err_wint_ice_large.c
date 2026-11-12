/* flags: --c-std=gnu23 */
/* expect-error: C3009 */
/* an array length wider than 64 bits */
int a[(unsigned __int128)1 << 64];
int main(void) { return 0; }
