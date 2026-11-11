/* flags: --c-std=c23 */
/* expect-error: C3073 */
constexpr int i = 1.0;
int main(void) { return 0; }
