/* flags: --c-std=c23 */
/* expect-error: C3075 */
enum E : int x;
int main(void) { return 0; }
