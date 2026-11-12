/* flags: --c-std=gnu11 */
/* expect-error: C3901 */
/* Inline assembly is not supported (yet). */
int main(void) { __asm__ volatile ("nop"); return 0; }
