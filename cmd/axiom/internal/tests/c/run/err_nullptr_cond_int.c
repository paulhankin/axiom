/* flags: --c-std=c23 */
/* expect-error: C3020 */
/* ?: takes nullptr_t with a pointer or nullptr_t, not with 0 */
int main(void) { int c = 1; int *p = c ? nullptr : 0; return p != 0; }
