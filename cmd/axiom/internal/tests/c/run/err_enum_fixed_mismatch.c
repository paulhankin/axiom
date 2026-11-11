/* flags: --c-std=c23 */
/* expect-error: C3075 */
enum E : int;
enum E : long { A };
int main(void) { return 0; }
