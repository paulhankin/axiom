/* flags: --c-std=c23 */
/* check-only */
/* The format checks of C23's wN length modifiers under scanf, whose C23
   form the header renames: a pointer to an object of another width is a
   mismatch (the message names scanf and uint16_t). */
/* expect-error: C4130 */
#include <stdint.h>
#include <stdio.h>
#pragma GCC diagnostic error "-Wformat"

int main(void) {
    unsigned long long v;
    return scanf("%w16u", &v);
}
