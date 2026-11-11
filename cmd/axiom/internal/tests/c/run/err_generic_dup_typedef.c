/* expect-error: C3202 */
typedef unsigned U;
int x = _Generic(1u, U: 1, unsigned int: 2);
int main(void) { return x; }
