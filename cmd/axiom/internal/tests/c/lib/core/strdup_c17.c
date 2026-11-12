/* strdup is C23's: <string.h> does not declare it in C17 mode (C3060). */
/* expect-error: C3060 */
#include <string.h>
int main(void) {
    return strdup("x") != 0;
}
