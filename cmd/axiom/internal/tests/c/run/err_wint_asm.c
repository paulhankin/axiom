/* flags: --c-std=gnu23 */
/* native-only */
/* expect-error: C3901 */
/* an asm operand of __int128 in a register: wider than a register */
void f(__int128 x) { __asm__ volatile("" : : "r"(x)); }
int main(void) { return 0; }
