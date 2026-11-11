/* expect-error: C3900 */
_Alignas(16) int x;
int main(void) { return x; }
