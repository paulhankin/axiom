/* flags: --c-std=c99 */
/* expect-error: C2001 */
struct P { int x; } p = { x: 1 };
int main(void) { return 0; }
