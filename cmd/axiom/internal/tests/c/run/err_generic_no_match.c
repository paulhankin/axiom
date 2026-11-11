/* expect-error: C3203 */
/* no default, and the controlling type (int, after lvalue conversion) matches none */
const int c = 1;
int x = _Generic(c, const int: 1, long: 2);
int main(void) { return x; }
