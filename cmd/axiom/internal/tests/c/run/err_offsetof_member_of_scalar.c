/* expect-error: C3081 */
/* A member designator that goes into a member that is not a structure. */
#include <stddef.h>
struct s { int a; };
int main(void) { return (int)offsetof(struct s, a.b); }
