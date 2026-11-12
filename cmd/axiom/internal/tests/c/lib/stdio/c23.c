/* C23's additions to <stdio.h>: __STDC_VERSION_STDIO_H__, %b and %B, the
   wN and wfN length modifiers, _PRINTF_NAN_LEN_MAX. */
/* flags: --c-std=c23 */
#include <stdio.h>
#include <stdint.h>

int main(void) {
    unsigned v = 0;
    printf("%ld %d\n", (long)__STDC_VERSION_STDIO_H__, _PRINTF_NAN_LEN_MAX);
    printf("%b %#b %#B %08b %.4b\n", 10u, 5u, 5u, 3u, 1u);
    printf("%w32d %wf16u\n", (int32_t)-1, (uint_fast16_t)65535);
    printf("%d %u\n", sscanf("0b1101 11", "%b %b", &v, &v), v);
    return 0;
}
/* expect: 202311 0 */
/* expect: 1010 0b101 0B101 00000011 0001 */
/* expect: -1 65535 */
/* expect: 2 3 */
