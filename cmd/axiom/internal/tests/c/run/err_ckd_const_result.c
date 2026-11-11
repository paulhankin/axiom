/* expect-error: C3082 */
/* ckd_add into a const object. */
/* flags: --c-std=c23 */
#include <stdckdint.h>
int main(void) { const int r = 0; return ckd_add(&r, 1, 2); }
