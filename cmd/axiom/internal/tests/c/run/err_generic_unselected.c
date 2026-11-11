/* expect-error: C3020 */
/* the associations that are not selected must be valid too */
int x = _Generic(1, int: 1, double: 2 * "a");
int main(void) { return x; }
