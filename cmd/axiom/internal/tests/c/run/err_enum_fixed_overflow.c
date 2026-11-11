/* flags: --c-std=c23 */
/* expect-error: C3013 */
enum E : unsigned char { A = 255, B };
int main(void) { return 0; }
