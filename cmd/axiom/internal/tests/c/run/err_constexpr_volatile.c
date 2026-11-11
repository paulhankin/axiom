/* flags: --c-std=c23 */
/* expect-error: C3073 */
constexpr volatile int v = 1;
int main(void) { return 0; }
