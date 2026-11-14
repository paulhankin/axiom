/* flags: --c-std=c17 */
/* expect-error: C3022 */
/* Under an ISO standard an assignment that drops the qualifiers of the
   pointed-to type is a constraint violation (6.5.16.1p1), here from the
   type of a conditional expression. */
int main(void) {
    void *p;
    int i = 0;
    p = i ? 0 : (const void *)0;
    return p != 0;
}
