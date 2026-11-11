/* flags: --c-std=c99 */
/* expect-error: C3221 */
/* an inline definition (no extern declaration) defines a modifiable static object */
inline int next(void) { static int n; return ++n; }
int main(void) { return 0; }
