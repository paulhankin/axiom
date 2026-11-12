/* The controlling expression of switch has integer type (6.8.4.2p1). */
/* expect-error: C3030 */
int main(void) { double _Complex z = 1; switch (z) { default: break; } return 0; }
