/* flags: --c-std=c99 */
/* expect-error: C2100 */
int x = _Generic(1, int: 1);
int main(void) { return x; }
