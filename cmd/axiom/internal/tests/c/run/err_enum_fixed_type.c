/* flags: --c-std=c23 */
/* expect-error: C3075 */
enum E : float { A };
int main(void) { return 0; }
