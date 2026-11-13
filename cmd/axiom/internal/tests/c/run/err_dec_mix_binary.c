/* flags: --c-std=c23 */
/* expect-error: C3230 */
int main(void) { _Decimal64 x = 1; double d = 2; x = x + d; return 0; }
