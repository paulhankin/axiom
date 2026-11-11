/* flags: --c-std=c99 --c-int=32 */
/* expect-error: C3036 */
/* 6.6p4: a constant expression's value is in range: a signed operation
   that overflows (with 32-bit int) in one where a constant is required. */
enum e { A = 65535 * 65535 };
int main(void) { return 0; }
