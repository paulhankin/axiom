/* flags: --c-std=c23 */
/* expect-error: C3005 */
_Complex _Decimal64 z;
int main(void) { return 0; }
