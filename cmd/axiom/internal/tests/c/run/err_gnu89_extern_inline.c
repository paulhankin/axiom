/* flags: --c-std=gnu89 */
/* expect-error: C3221 */
/* In gnu89 `extern inline` makes the inline definition (6.7.4p3 holds). */
extern inline int counter(void) {
    static int n;
    return ++n;
}
int main(void) { return counter(); }
