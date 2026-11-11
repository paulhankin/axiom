/* flags: --c-std=c23 */
/* expect-error: C3077 */
int main(void) { return (int)sizeof(int [[nodiscard]]); }
