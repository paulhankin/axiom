/* flags: --c-std=gnu17 */
/* expect-error: C3022 */
/* A GNU dialect lets an assignment drop const, volatile and restrict from
   the pointed-to type with a warning, but not _Atomic, whose accesses are
   not the plain type's: still a constraint violation. */
int main(void) {
    static _Atomic int a;
    const _Atomic int *pa = &a;
    int *p = pa;
    return p != 0;
}
