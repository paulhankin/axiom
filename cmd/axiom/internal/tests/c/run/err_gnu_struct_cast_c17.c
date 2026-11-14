/* flags: --c-std=c17 */
/* expect-error: C3028 */
/* Under an ISO standard a cast is to void or a scalar type (6.5.4p2), even
   to the operand's own structure type. */
struct s { int a; };
int main(void) {
    struct s x = { 1 };
    struct s y = (struct s)x;
    return y.a;
}
