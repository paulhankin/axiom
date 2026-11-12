/* flags: --c-std=c11 */
/* expect-error: C3020 */
/* compound assignment constraints hold for atomic operands too (6.5.16.2) */
_Atomic double d;
int main(void) { d %= 2; return 0; }
