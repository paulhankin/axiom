/* flags: --c-std=c17 */
/* expect-error: C3011 */
/* Under an ISO standard an enumeration is named only after its list
   (6.7.2.3p3): a forward declaration is a constraint violation. */
enum later;
enum later { ONE };
int main(void) { return ONE; }
