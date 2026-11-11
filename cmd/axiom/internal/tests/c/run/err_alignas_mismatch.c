/* expect-error: C3071 */
_Alignas(8) int x;
_Alignas(16) int x;
int main(void) { return 0; }
