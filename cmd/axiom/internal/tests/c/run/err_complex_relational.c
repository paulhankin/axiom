/* Complex numbers are not ordered: no relational operators (6.5.8p2). */
/* expect-error: C3020 */
int main(void) { double _Complex z = 1; return z < 2; }
