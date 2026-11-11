/* flags: --c-std=c23 */
/* expect-error: C2001 */
/* a type name as the controlling operand is not C23 */
int x = _Generic(int, int: 1);
int main(void) { return x; }
