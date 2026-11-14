/* flags: --c-std=c23 */
/* The format checks of C23's wN length modifiers: an argument of 8 or 16
   bits arrives promoted to int, as for hh and h (7.23.6.1p7), and is no
   mismatch; the format checks are errors here. */
#include <stdint.h>
#include <stdio.h>
#pragma GCC diagnostic error "-Wformat"

int main(void) {
    int8_t a = -8;
    uint8_t b = 200;
    int16_t c = -300;
    uint16_t d = 65535;
    printf("%w8d %w8u %w16d %w16u %w8x\n", a, b, c, d, (uint8_t)0xAB); /* expect: -8 200 -300 65535 ab */
    int8_t sa;
    uint16_t sd;
    int n = sscanf("-5 7", "%w8d %w16u", &sa, &sd);
    printf("%d %d %d\n", n, sa, sd); /* expect: 2 -5 7 */
    return 0;
}
