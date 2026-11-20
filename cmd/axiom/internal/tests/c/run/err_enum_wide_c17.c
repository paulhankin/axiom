/* flags: --c-std=c17 */
/* expect-error: C3013 */
/* Before C23, under an ISO standard, an enumeration constant is an int. */
enum E { WIDE = (long long)1 << 40 };
int main(void) { return 0; }
