/* flags: --c-std=c23 */
/* expect-error: C3073 */
constexpr signed char s[] = "\xff";
int main(void) { return 0; }
