/* expect-error: C3020 */
/* both operands of ?: are void, or neither */
int main(void) { int c = 1; int x = 2; c ? (void)0 : x; return 0; }
