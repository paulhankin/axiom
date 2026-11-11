/* flags: --c-std=c23 */
/* expect-error: C3073 */
thread_local constexpr int t = 1;
int main(void) { return 0; }
