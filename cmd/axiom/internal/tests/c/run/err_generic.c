/* expect-error: C3900 */
int main(void) { return _Generic(1, int: 0, default: 1); }
