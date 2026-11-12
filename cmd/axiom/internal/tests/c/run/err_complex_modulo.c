/* % takes integer operands (6.5.5p2). */
/* expect-error: C3020 */
int main(void) { double _Complex z = 1; z = z % 2; return 0; }
