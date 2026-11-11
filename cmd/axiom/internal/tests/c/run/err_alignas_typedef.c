/* expect-error: C3071 */
typedef _Alignas(8) int T;
int main(void) { return 0; }
