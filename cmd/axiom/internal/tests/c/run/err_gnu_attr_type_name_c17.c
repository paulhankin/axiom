/* flags: --c-std=c17 */
/* expect-error: C2001 */
/* Under an ISO standard __attribute__ is an identifier: it cannot begin a
   type name. */
int main(void) {
    long x = 1;
    return (int)sizeof(__attribute__((unused)) int) + (int)x;
}
