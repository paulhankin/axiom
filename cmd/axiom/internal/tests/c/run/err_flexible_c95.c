/* flags: --c-std=c95 */
/* expect-error: C3007 */
/* Flexible array members are C99's: before, a member of incomplete type. */
struct s { int n; int a[]; };
int main(void) { return 0; }
