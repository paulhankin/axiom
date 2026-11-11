/* expect-error: C3081 */
/* offsetof of a bit-field. */
#include <stddef.h>
struct s { int a : 3; int b; };
int main(void) { return (int)offsetof(struct s, a); }
