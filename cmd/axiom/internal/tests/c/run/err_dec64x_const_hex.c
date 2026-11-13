/* A d64x suffix is not one of a hexadecimal constant (H.5.2p3). */
/* flags: --c-std=c23 */
/* expect-error: C1019 */
int main(void) { _Decimal64x x = 0x1p3d64x; return (int)x; }
