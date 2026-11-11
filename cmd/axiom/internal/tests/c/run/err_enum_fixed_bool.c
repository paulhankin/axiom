/* flags: --c-std=c23 */
/* expect-error: C3013 */
enum E : bool { A, B, C };
int main(void) { return 0; }
