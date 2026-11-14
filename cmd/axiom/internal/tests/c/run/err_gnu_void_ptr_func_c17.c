/* flags: --c-std=c17 */
/* expect-error: C3022 */
/* Under an ISO standard void * converts to a pointer to a function only
   with a cast (6.5.16.1p1). */
static void call(void *p) {
    void (*f)(void) = p;
    f();
}
static void g(void) { }
int main(void) { call((void *)0); (void)g; return 0; }
