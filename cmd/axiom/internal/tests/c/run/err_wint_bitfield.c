/* flags: --c-std=gnu23 */
/* expect-error: C3901 */
/* a bit-field of a packed structure that no unit of 8 bytes holds */
struct __attribute__((packed)) s { char c; __int128 x : 100; };
int main(void) { return 0; }
