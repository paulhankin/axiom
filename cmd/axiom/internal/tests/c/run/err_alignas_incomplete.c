/* expect-error: C3071 */
struct S;
_Alignas(struct S) int x;
int main(void) { return 0; }
