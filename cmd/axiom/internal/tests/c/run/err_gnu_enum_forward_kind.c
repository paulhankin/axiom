/* flags: --c-std=gnu17 */
/* expect-error: C3011 */
/* A GNU dialect's forward declaration of an enumeration is of the tag's
   kind: the tag of a structure is not an enumeration's. */
struct thing { int a; };
enum thing *p;
int main(void) { return p != 0; }
