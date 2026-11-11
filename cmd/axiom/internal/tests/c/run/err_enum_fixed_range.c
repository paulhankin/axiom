/* flags: --c-std=c23 */
/* expect-error: C3013 */
enum E : unsigned char { A = 256 };
int main(void) { return 0; }
