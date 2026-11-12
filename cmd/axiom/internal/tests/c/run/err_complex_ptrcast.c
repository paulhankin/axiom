/* A complex value cannot be cast to a pointer (6.5.4p4). */
/* expect-error: C3028 */
int main(void) { double _Complex z = 1; char *p = (char *)z; return p != 0; }
