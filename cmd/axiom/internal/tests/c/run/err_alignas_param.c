/* expect-error: C3071 */
void f(_Alignas(8) int x);
int main(void) { return 0; }
