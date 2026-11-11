/* Case values are compared after conversion to the promoted controlling type. */
/* expect-error: C3041 */
int f(unsigned x) { switch (x) { case -1: return 1; case 0xFFFFFFFFu: return 2; } return 0; }
int main(void) { return f(0); }
