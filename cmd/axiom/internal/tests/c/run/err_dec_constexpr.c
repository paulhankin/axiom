/* flags: --c-std=c23 */
/* expect-error: C3073 */
constexpr _Decimal32 c = 1.23456789dd;
int main(void) { return 0; }
