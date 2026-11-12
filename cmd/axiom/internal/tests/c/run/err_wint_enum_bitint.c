/* flags: --c-std=c23 */
/* expect-error: C3075 */
/* the underlying type of an enumeration cannot be a bit-precise integer type (6.7.2.2p5) */
enum e : _BitInt(100) { A };
int main(void) { return 0; }
