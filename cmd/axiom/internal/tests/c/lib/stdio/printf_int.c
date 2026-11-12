/* fprintf's integer conversions (C17 7.21.6.1): d i o u x X with every
   flag, field widths and precisions (given and by *), the length
   modifiers hh h l ll j z t, and the C23 conversion b B (accepted in
   every mode: before C23 it is undefined). */
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

static const char *const fmts[] = {
    "%d", "%5d", "%-5d|", "%05d", "%+d", "% d", "%.3d", "%8.3d", "%-8.3d|", "%08.3d", "%.0d", "%+.0d", "%i",
    "%o", "%#o", "%#.0o", "%#5o", "%u", "%x", "%#x", "%#X", "%#08x", "%-#8x|", "%.4x", "%#.4X", "%b", "%#b", "%#B",
    "%010b", "% 05d", "%+05d", "%-+5d|",
};
static const int vals[] = {0, 1, -1, 42, -42, 255, 32767, -32768};

int main(void) {
    size_t i, j;
    for (i = 0; i < sizeof fmts / sizeof fmts[0]; i++) {
        printf("%-9s", fmts[i]);
        for (j = 0; j < sizeof vals / sizeof vals[0]; j++) {
            putchar('[');
            printf(fmts[i], vals[j]);
            putchar(']');
        }
        putchar('\n');
    }
    /* the extremes of each length */
    printf("%hhd %hhu %hhx %hd %hu\n", 200, 200, -1, 40000, -1);
    printf("%ld %lu %lx\n", -2147483647L - 1, 4294967295UL, 0x7fffffffUL);
    printf("%lld %llu %llx %llo\n", -9223372036854775807LL - 1, 18446744073709551615ULL, 0xfedcba9876543210ULL,
           01777777777777777777777ULL);
    printf("%jd %ju %zu %zx %td\n", (intmax_t)INT64_MIN, (uintmax_t)UINT64_MAX, (size_t)12345, (size_t)0xbeef,
           (ptrdiff_t)-77);
    /* * widths and precisions: a negative width is the - flag, a negative
       precision is none */
    printf("[%*d][%-*d][%*d][%.*d][%.*d][%*.*d]\n", 6, 1, 6, 2, -6, 3, 4, 5, -4, 6, 7, 3, 8);
    /* C23 wN and wfN */
    printf("%w8d %w16d %w32x %w64d %wf8u %wf16d %wf32d %wf64x\n", (int8_t)-5, (int16_t)-300, (uint32_t)0xabcdef01u,
           (int64_t)-1234567890123LL, (uint_fast8_t)250, (int_fast16_t)-2, (int_fast32_t)77,
           (uint_fast64_t)0xffffffffffULL);
    /* the count %n stores, at each length */
    {
        int n = 0;
        signed char hh = 0;
        short h = 0;
        long l = 0;
        long long ll = 0;
        size_t z = 0;
        printf("abc%n%hhn%hn%ln%lln%zn|\n", &n, &hh, &h, &l, &ll, &z);
        printf("%d %d %d %ld %lld %zu\n", n, hh, h, l, ll, z);
    }
    printf("%%|%5%|%-5d%%\n", 3);
    return 0;
}
/* expect: %d       [0][1][-1][42][-42][255][32767][-32768] */
/* expect: %5d      [    0][    1][   -1][   42][  -42][  255][32767][-32768] */
/* expect: %-5d|    [0    |][1    |][-1   |][42   |][-42  |][255  |][32767|][-32768|] */
/* expect: %05d     [00000][00001][-0001][00042][-0042][00255][32767][-32768] */
/* expect: %+d      [+0][+1][-1][+42][-42][+255][+32767][-32768] */
/* expect: % d      [ 0][ 1][-1][ 42][-42][ 255][ 32767][-32768] */
/* expect: %.3d     [000][001][-001][042][-042][255][32767][-32768] */
/* expect: %8.3d    [     000][     001][    -001][     042][    -042][     255][   32767][  -32768] */
/* expect: %-8.3d|  [000     |][001     |][-001    |][042     |][-042    |][255     |][32767   |][-32768  |] */
/* expect: %08.3d   [     000][     001][    -001][     042][    -042][     255][   32767][  -32768] */
/* expect: %.0d     [][1][-1][42][-42][255][32767][-32768] */
/* expect: %+.0d    [+][+1][-1][+42][-42][+255][+32767][-32768] */
/* expect: %i       [0][1][-1][42][-42][255][32767][-32768] */
/* expect: %o       [0][1][37777777777][52][37777777726][377][77777][37777700000] */
/* expect: %#o      [0][01][037777777777][052][037777777726][0377][077777][037777700000] */
/* expect: %#.0o    [0][01][037777777777][052][037777777726][0377][077777][037777700000] */
/* expect: %#5o     [    0][   01][037777777777][  052][037777777726][ 0377][077777][037777700000] */
/* expect: %u       [0][1][4294967295][42][4294967254][255][32767][4294934528] */
/* expect: %x       [0][1][ffffffff][2a][ffffffd6][ff][7fff][ffff8000] */
/* expect: %#x      [0][0x1][0xffffffff][0x2a][0xffffffd6][0xff][0x7fff][0xffff8000] */
/* expect: %#X      [0][0X1][0XFFFFFFFF][0X2A][0XFFFFFFD6][0XFF][0X7FFF][0XFFFF8000] */
/* expect: %#08x    [00000000][0x000001][0xffffffff][0x00002a][0xffffffd6][0x0000ff][0x007fff][0xffff8000] */
/* expect: %-#8x|   [0       |][0x1     |][0xffffffff|][0x2a    |][0xffffffd6|][0xff    |][0x7fff  |][0xffff8000|] */
/* expect: %.4x     [0000][0001][ffffffff][002a][ffffffd6][00ff][7fff][ffff8000] */
/* expect: %#.4X    [0000][0X0001][0XFFFFFFFF][0X002A][0XFFFFFFD6][0X00FF][0X7FFF][0XFFFF8000] */
/* expect: %b       [0][1][11111111111111111111111111111111][101010][11111111111111111111111111010110][11111111][111111111111111][11111111111111111000000000000000] */
/* expect: %#b      [0][0b1][0b11111111111111111111111111111111][0b101010][0b11111111111111111111111111010110][0b11111111][0b111111111111111][0b11111111111111111000000000000000] */
/* expect: %#B      [0][0B1][0B11111111111111111111111111111111][0B101010][0B11111111111111111111111111010110][0B11111111][0B111111111111111][0B11111111111111111000000000000000] */
/* expect: %010b    [0000000000][0000000001][11111111111111111111111111111111][0000101010][11111111111111111111111111010110][0011111111][111111111111111][11111111111111111000000000000000] */
/* expect: % 05d    [ 0000][ 0001][-0001][ 0042][-0042][ 0255][ 32767][-32768] */
/* expect: %+05d    [+0000][+0001][-0001][+0042][-0042][+0255][+32767][-32768] */
/* expect: %-+5d|   [+0   |][+1   |][-1   |][+42  |][-42  |][+255 |][+32767|][-32768|] */
/* expect: -56 200 ff -25536 65535 */
/* expect: -2147483648 4294967295 7fffffff */
/* expect: -9223372036854775808 18446744073709551615 fedcba9876543210 1777777777777777777777 */
/* expect: -9223372036854775808 18446744073709551615 12345 beef -77 */
/* expect: [     1][2     ][3     ][0005][6][    008] */
/* expect: -5 -300 abcdef01 -1234567890123 250 -2 77 ffffffffff */
/* expect: abc| */
/* expect: 3 3 3 3 3 3 */
/* expect: %|%|3    % */
