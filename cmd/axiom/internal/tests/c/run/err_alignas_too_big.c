/* expect-error: C3071 */
_Alignas(1 << 20) char x;
int main(void) { return 0; }
