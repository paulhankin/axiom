/* expect-error: C3020 */
int main(void) { int *p = 0, *q = 0; return p + q != 0; }
