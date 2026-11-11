/* flags: --c-std=c23 */
/* expect-error: C3210 */
unsigned _BitInt(0) x;
int main(void) { return 0; }
