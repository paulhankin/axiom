/* expect-error: C3035 */
/* A subscript in offsetof that is not an integer constant expression. */
#include <stddef.h>
struct s { int a[4]; };
int main(void) { int i = 1; return (int)offsetof(struct s, a[i]); }
