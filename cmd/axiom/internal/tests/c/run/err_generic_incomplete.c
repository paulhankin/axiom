/* expect-error: C3201 */
struct S;
int x = _Generic(1, struct S: 1, int: 2);
int main(void) { return x; }
