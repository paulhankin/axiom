/* flags: --c-std=c23 */
/* expect-error: C3002 */
enum E { A, B };
enum E { A, B, C };
int main(void) { return 0; }
