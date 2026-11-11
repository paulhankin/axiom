/* flags: --c-std=c23 */
/* expect-error: C3210 */
/* BITINT_MAXWIDTH is 64 */
_BitInt(65) x;
int main(void) { return 0; }
