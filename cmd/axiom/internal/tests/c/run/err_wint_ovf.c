/* flags: --c-std=gnu23 */
/* expect-error: C3036 */
/* a signed overflow in a constant expression of 128 bits */
static __int128 x = ((__int128)1 << 126) * 2;
int main(void) { return 0; }
