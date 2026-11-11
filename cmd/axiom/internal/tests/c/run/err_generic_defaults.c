/* expect-error: C3200 */
int x = _Generic(1, int: 1, default: 2, default: 3);
int main(void) { return x; }
