/* flags: --c-std=c23 */
/* expect-error: C3077 */
int *[[maybe_unused]] p;
int main(void) { return 0; }
