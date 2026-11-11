/* flags: --c-std=c89 */
/* expect-error: C2108 */
/* expect-error: C2108 */
struct s { int a, b; } v = { .b = 1, .a = 2 };
int w[3] = { [2] = 1 };
