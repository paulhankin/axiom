/* flags: --c-std=gnu99 */
/* expect-error: C3044 */
/* From gnu99 on a return with a value in a function returning void stays
   an error (gnu89 takes it with the warning C4146). */
static int g(void) { return 1; }
static void f(void) { return g(); }
int main(void) { f(); return 0; }
