/* flags: --c-std=c23 */
/* expect-error: C3077 */
/* An attribute declaration appertains to nothing. */
[[maybe_unused]];
int main(void) { return 0; }
