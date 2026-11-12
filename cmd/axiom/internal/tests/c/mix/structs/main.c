/* Structures by value between translation units, as the C ABI passes them
   (small ones in registers, large ones in memory, those of floating members
   in floating-point registers where the ABI says), and the same structures
   by pointer between C and Axiom, whose struct layout is C's (rt.ax). */
/* uses: ../print.ax */
#include "../print.h"
#include "s.h"

int main(void) {
    struct small s = make_small(3, -4);
    print_i64(s.a * 10 + s.b); /* expect: 26 */
    struct large l = make_large(5);
    print_i64(sum_large(l)); /* expect: 75 */
    struct fl f = scale_fl((struct fl){1.5f, 2.5f}, 2.0f);
    print_f64(f.x + f.y); /* expect: 8 */
    struct dd d = swap_dd((struct dd){0.25, 4.0});
    print_f64(d.x - d.y); /* expect: 3.75 */
    struct mixed m = make_mixed(9);
    print_f64(m.d + m.i + m.c); /* expect: 37.5 */
    /* the same layouts in Axiom */
    print_i64(ax_small_sum(&s)); /* expect: 8 */
    print_i64(ax_large_sum(&l)); /* expect: 75 */
    print_f64(ax_mixed_sum(&m)); /* expect: 37.5 */
    ax_mixed_set(&m);
    print_f64(m.d); /* expect: -1.5 */
    print_i64(m.i); /* expect: 77 */
    print_i64(m.c); /* expect: 66 */
    return 0;
}
