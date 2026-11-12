/* <<= takes integer operands (6.5.16.2p2). */
/* expect-error: C3020 */
int main(void) { double _Complex z = 1; z <<= 1; return 0; }
