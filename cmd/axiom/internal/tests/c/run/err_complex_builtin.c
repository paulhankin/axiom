/* __builtin_complex takes two operands of one real floating type. */
/* expect-error: C3082 */
int main(void) { double _Complex z = __builtin_complex(1, 2); return 0; }
