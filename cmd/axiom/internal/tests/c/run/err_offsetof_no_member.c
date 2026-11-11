/* expect-error: C3025 */
/* offsetof of a member the structure does not have. */
#include <stddef.h>
struct s { int a; };
int main(void) { return (int)offsetof(struct s, b); }
