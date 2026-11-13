/* flags: --c-std=gnu23 */
/* expect-error: C3002 */
/* A list again with a value that differs only above the first 64 bits. */
enum W : __int128 { A = (__int128)1 << 64 };
enum W : __int128 { A = (__int128)1 << 65 };
int main(void) { return 0; }
