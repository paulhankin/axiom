/* flags: --c-std=gnu23 */
/* expect-error: C3212 */
/* an integer constant expression wider than 64 bits where a value is needed */
_Alignas((unsigned __int128)1 << 70) int x;
int main(void) { return 0; }
