/* expect-error: C3020 */
int main(void) { int c = 1; int *p = 0; long *q = 0; return (c ? p : q) == 0; }
