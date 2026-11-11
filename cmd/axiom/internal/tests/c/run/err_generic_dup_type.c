/* expect-error: C3202 */
/* signed int and int are the same type */
int x = _Generic(1, int: 1, signed int: 2);
int main(void) { return x; }
