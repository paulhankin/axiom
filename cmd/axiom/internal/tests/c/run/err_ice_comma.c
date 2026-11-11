/* expect-error: C3035 */
/* a constant expression has no comma operator (6.6p3) */
enum { A = (1, 2) };
int main(void) { return A; }
