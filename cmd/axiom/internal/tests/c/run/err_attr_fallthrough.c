/* flags: --c-std=c23 */
/* expect-error: C3077 */
[[fallthrough]] int a;
int main(void) { return 0; }
