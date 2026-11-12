/* flags: --c-std=gnu23 */
/* expect-error: C3901 */
enum e : __int128 { A };
int main(void) { return 0; }
