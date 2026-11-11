/* flags: --c-std=c23 */
/* expect-error: C3013 */
enum E { A = 0x7FFFFFFFFFFFFFFF, B };
int main(void) { return 0; }
