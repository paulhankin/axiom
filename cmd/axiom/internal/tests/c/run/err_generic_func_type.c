/* expect-error: C3201 */
int x = _Generic(1, int(void): 1, int: 2);
int main(void) { return x; }
