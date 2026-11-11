/* flags: --c-std=c23 */
/* expect-error: C3022 */
/* Structures without a tag in different scopes are not compatible, even with the same members. */
int main(void) {
    struct { int x; } *p = 0;
    {
        struct { int x; } *q = p;
        (void)q;
    }
    return 0;
}
