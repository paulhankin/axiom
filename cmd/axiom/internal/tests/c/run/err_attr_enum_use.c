/* flags: --c-std=c23 */
/* expect-error: C3077 */
enum E { A };
enum [[maybe_unused]] E e;
int main(void) { return 0; }
