/* expect-error: C3013 */
enum E { A = 2147483647, B };
int main(void) { return 0; }
