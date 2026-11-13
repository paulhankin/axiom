/* flags: --c-std=gnu23 */
/* native-only */
/* expect-error: C3096 */
/* an asm operand of __int128 in a register: wider than a register: memory or x86-64's "A" holds it */
void f(__int128 x) { __asm__ volatile("" : : "r"(x)); }
int main(void) { return 0; }
