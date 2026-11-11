/* flags: --c-std=c23 */
/* expect-error: C3075 */
enum F { X };
enum E : enum F;
int main(void) { return 0; }
