/* flags: --c-std=c23 */
/* expect-error: C3210 */
/* a signed _BitInt needs at least 2 bits */
_BitInt(1) x;
int main(void) { return 0; }
