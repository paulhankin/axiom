/* expect-error: C2001 */
struct s { int a; } v;
int f(void) { return v.; }
