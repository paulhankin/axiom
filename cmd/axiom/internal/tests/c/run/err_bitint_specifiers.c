/* flags: --c-std=c23 */
/* expect-error: C3005 */
long _BitInt(8) x;
int main(void) { return 0; }
