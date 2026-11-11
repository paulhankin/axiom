/* flags: --c-std=c23 */
/* expect-error: C3077 */
[[noreturn()]] void f(void);
int main(void) { return 0; }
