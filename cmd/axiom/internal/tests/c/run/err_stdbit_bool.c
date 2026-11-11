/* expect-error: C3082 */
/* A <stdbit.h> macro on bool. */
/* flags: --c-std=c23 */
#include <stdbit.h>
int main(void) { return (int)stdc_count_ones(true); }
