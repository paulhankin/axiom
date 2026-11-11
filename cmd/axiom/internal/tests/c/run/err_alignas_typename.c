/* expect-error: C3071 */
unsigned long n = sizeof(_Alignas(8) int);
int main(void) { return 0; }
