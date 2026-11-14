/* flags: --c-std=c17 */
/* expect-error: C3020 */
/* Under an ISO standard the operands of '?:' are both void or neither
   (6.5.15p3). */
static void f(void) { }
int main(void) {
    int i = 1;
    i ? i : f();
    return 0;
}
