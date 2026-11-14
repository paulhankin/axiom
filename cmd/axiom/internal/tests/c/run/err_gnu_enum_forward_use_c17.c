/* flags: --c-std=c17 */
/* expect-error: C3011 */
/* Under an ISO standard a pointer to an enumeration whose list is not
   given is a constraint violation (6.7.2.3p3). */
enum later *p;
int main(void) { return p != 0; }
