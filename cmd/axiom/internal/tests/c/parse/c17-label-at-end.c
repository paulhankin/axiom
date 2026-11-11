/* flags: --c-std=c17 */
/* expect-error: C2138 */
void f(void) {
    goto l;
l:
}
