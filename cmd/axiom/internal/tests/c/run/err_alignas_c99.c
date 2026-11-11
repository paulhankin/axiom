/* flags: --c-std=c99 */
/* expect-error: C2100 */
_Alignas(4) int x;
int main(void) { return 0; }
