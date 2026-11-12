/* flags: --c-std=c23 */
/* expect-error: C3210 */
/* BITINT_MAXWIDTH is 65535 */
_BitInt(65536) x;
int main(void) { return 0; }
