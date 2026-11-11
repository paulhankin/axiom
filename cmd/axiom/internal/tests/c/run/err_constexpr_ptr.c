/* flags: --c-std=c23 */
/* expect-error: C3073 */
int x;
constexpr int *p = &x;
int main(void) { return 0; }
