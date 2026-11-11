/* flags: --c-std=c11 */
/* Character arrays initialized by string literals of each kind: wchar_t
   from L"", char16_t from u"", char32_t from U"", char from u8"" (and
   plain ""), with the length from the string, braces around it, or a
   longer array (zeros after it); nested in structures and arrays. */
#include "host.h"
#include <stddef.h>

typedef unsigned short char16;
typedef unsigned int char32;

static wchar_t ws[] = L"wide";
static char16 s16[8] = u"abc";
static char32 s32[] = { U"\x10FFFF!" };
static char u8s[] = u8"é";
static struct { int n; wchar_t name[4]; } named[] = { { 1, L"ab" }, { 2, L"xyz" } };

int main(void) {
    wchar_t lw[6] = L"hi";
    char16 l16[] = u"☺";
    char32 l32[3] = U"Z";
    char plain[2][3] = { "ab", "c" };
    print_i64(sizeof ws / sizeof ws[0]);
    print_i64(ws[0] + ws[4]);
    print_i64(s16[2] + s16[3] + s16[7]);
    print_i64(sizeof s32 / sizeof s32[0]);
    print_u64(s32[0]);
    print_i64(sizeof u8s);
    print_i64((unsigned char)u8s[0]);
    print_i64(named[1].name[2] + named[0].name[3]);
    print_i64(lw[1] + lw[5]);
    print_i64(sizeof l16 / sizeof l16[0]);
    print_i64(l16[0]);
    print_i64(l32[0] + l32[2]);
    print_i64(plain[1][0] + plain[1][1]);
    return 0;
}
/* expect: 5 */
/* expect: 119 */
/* expect: 99 */
/* expect: 3 */
/* expect: 1114111 */
/* expect: 3 */
/* expect: 195 */
/* expect: 122 */
/* expect: 105 */
/* expect: 2 */
/* expect: 9786 */
/* expect: 90 */
/* expect: 99 */
