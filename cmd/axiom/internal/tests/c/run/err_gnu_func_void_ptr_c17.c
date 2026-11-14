/* flags: --c-std=c17 */
/* expect-error: C3022 */
/* Under an ISO standard a function pointer converts to void * only with a
   cast (6.5.16.1p1): an initialization without one is a constraint
   violation. */
static void f(void) { }
void *p = f;
int main(void) { return p != 0; }
