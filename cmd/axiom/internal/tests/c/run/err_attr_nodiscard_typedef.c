/* flags: --c-std=c23 */
/* expect-error: C3077 */
[[nodiscard]] typedef int T;
int main(void) { return 0; }
