/* _Decimal64x and a binary floating operand have no common type (H.4.3). */
/* flags: --c-std=c23 */
/* expect-error: C3230 */
int main(void) { _Decimal64x x = 1; double f = 2; x = x + f; return 0; }
