/* ~ takes an integer operand: it is not the complex conjugate in ISO C (6.5.3.3p1). */
/* expect-error: C3020 */
int main(void) { double _Complex z = 1; z = ~z; return 0; }
