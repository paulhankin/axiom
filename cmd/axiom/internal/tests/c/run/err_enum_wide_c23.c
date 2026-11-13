/* flags: --c-std=c23 */
/* expect-error: C3013 */
/* Under an ISO standard there is no integer type wider than long long for
   an enumeration's values (a bit-precise one cannot be its type). */
enum E { WIDE = (unsigned _BitInt(100))1 << 70 };
int main(void) { return 0; }
