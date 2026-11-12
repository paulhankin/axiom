/* A static complex object is initialized by a constant. */
/* expect-error: C3031 */
double _Complex f(void); static double _Complex z = f();
