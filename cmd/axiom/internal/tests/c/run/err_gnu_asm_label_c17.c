/* flags: --c-std=c17 */
/* expect-error: C2001 */
int f(void) __asm__("g");
int main(void) { return 0; }
