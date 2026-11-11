/* flags: --c-std=c23 */
/* expect-error: C3077 */
[[maybe_unused(1)]] int a;
int main(void) { return 0; }
