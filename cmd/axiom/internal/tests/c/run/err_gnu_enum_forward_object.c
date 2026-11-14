/* flags: --c-std=gnu17 */
/* expect-error: C3007 */
/* A GNU dialect's enumeration named before its list is incomplete until
   then: no object of it can be defined. */
enum later;
static int use(void) { enum later x; return 0; }
enum later { ONE };
int main(void) { return use(); }
