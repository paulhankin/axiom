/* ++ and -- take real or pointer operands, not complex ones (6.5.2.4p1). */
/* expect-error: C3020 */
int main(void) { double _Complex z = 1; z++; return 0; }
