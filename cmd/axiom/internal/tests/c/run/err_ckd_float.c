/* expect-error: C3082 */
/* ckd_sub of a floating operand. */
/* flags: --c-std=c23 */
#include <stdckdint.h>
int main(void) { int r; return ckd_sub(&r, 1.5, 2); }
