/* flags: --c-std=c23 */
/* expect-error: C3077 */
/* nodiscard appertains to functions and to structure, union and enumeration types (6.7.13). */
[[nodiscard]] int x;
int main(void) { return x; }
