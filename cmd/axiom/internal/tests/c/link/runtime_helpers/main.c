/* What objects of the system's C compiler need of its compiler's run-time
   library (complex multiplication, 128-bit division and conversions, and
   on AArch64 the atomics it calls out of line): its archives are linked as
   the system's linker links them, statically. */
/* cc-variants: -O2 | -O0 */
#include <stdio.h>

int sys_atomics(int n);
long long sys_int128(long long a, long long b);
double sys_complex(double a, double b);

int main(void) {
    printf("%d\n", sys_atomics(5)); /* expect: 10 */
    printf("%lld\n", sys_int128(123456789012345, 1000)); /* expect: 17636684144620713 */
    printf("%.1f\n", sys_complex(3.0, 4.0)); /* expect: -7.0 */
    return 0;
}
