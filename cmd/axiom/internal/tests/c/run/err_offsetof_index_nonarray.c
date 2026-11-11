/* expect-error: C3081 */
/* A subscript in offsetof on a member that is not an array. */
#include <stddef.h>
struct s { int a; };
int main(void) { return (int)offsetof(struct s, a[1]); }
