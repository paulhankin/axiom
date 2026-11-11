/* flags: --c-std=c17 */
/* expect-error: C2100 */
_BitInt(8) x;
int main(void) { return 0; }
