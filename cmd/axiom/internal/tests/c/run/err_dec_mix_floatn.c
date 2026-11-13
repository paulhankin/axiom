/* A decimal floating operand with an interchange binary one has no common
   type (C23 6.3.1.8p1), as with a standard floating one. */
/* flags: --c-std=c23 */
/* expect-error: C3230 */
int main(void) { _Decimal64 x = 1; _Float32 f = 2; x = x * f; return 0; }
