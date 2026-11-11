/* expect-error: C3082 */
/* ckd_mul of a plain char operand. */
/* flags: --c-std=c23 */
#include <stdckdint.h>
int main(void) { int r; char c = 1; return ckd_mul(&r, c, 2); }
