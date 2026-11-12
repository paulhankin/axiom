/* A complex value is not assigned to a pointer. */
/* expect-error: C3022 */
int main(void) { double _Complex z = 1; int *p = z; return 0; }
