/* expect-error: C3025 */
/* offsetof of a type that is not a structure or union. */
#include <stddef.h>
int main(void) { return (int)offsetof(int, x); }
