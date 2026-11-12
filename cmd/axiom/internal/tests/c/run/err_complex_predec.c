/* Prefix -- of a complex operand (6.5.3.1p1). */
/* expect-error: C3020 */
int main(void) { float _Complex z = 1; --z; return 0; }
