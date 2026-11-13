/* flags: --c-std=c23 */
/* expect-error: C1019 */
int main(void) { _Decimal64 x = 0x1p3dd; return (int)x; }
