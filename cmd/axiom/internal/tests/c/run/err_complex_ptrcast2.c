/* A pointer cannot be cast to a complex type (6.5.4p4). */
/* expect-error: C3028 */
int main(void) { char *p = 0; double _Complex z = (double _Complex)p; return 0; }
