/* flags: --c-std=gnu17 */
/* expect-error: C3029 */
/* A GNU dialect's enumeration named before its list is incomplete until
   then: its size is not known. */
enum later;
int n = sizeof(enum later);
enum later { ONE };
int main(void) { return n; }
