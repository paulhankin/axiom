/* flags: --c-std=c11 --c-char=signed */
/* The types and values of character constants and string literals
   (6.4.4.4, 6.4.5): a plain character constant is an int whose value is
   that of a char (so '\xff' is -1 when char is signed); L'', u'', U'' are
   wchar_t, char16_t and char32_t; u8"" is an array of char before C23;
   adjacent literals concatenate, a prefix applying to the result. */
#include <stddef.h>
#include "host.h"

#define T(x) _Generic((x), char *: 1, unsigned char *: 2, int: 3, unsigned short: 4, unsigned: 5, \
    unsigned short *: 6, unsigned *: 7, default: 0)

int main(void) {
    print_i64(T('a'));
    print_i64(sizeof('a') == sizeof(int));
    print_i64('\xff');
    print_i64('\377' == (char)255);
    print_i64('\0');
    print_i64(T(u'a'));
    print_i64(T(U'a'));
    print_i64(_Generic(L'a', wchar_t: 1, default: 0));
    print_i64(u'\xffff');
    print_i64(U'\U0010FFFF');
    print_i64(T(u8"x"));
    print_i64(T("x"));
    print_i64(T(u"x"));
    print_i64(T(U"x"));
    print_i64(sizeof("ab" "cd"));
    print_i64(sizeof(u"ab" "cd") / sizeof(u'a'));
    print_i64(sizeof("ab" U"cd") / sizeof(U'a'));
    print_i64(sizeof(u8"é") );
    print_i64(sizeof(L"x" L"y") / sizeof(wchar_t));
    print_i64("abc"[1]);
    print_i64(*("xyz" + 2));
    print_i64(sizeof "" );
    return 0;
}
/* expect: 3 */
/* expect: 1 */
/* expect: -1 */
/* expect: 1 */
/* expect: 0 */
/* expect: 4 */
/* expect: 5 */
/* expect: 1 */
/* expect: 65535 */
/* expect: 1114111 */
/* expect: 1 */
/* expect: 1 */
/* expect: 6 */
/* expect: 7 */
/* expect: 5 */
/* expect: 5 */
/* expect: 5 */
/* expect: 3 */
/* expect: 3 */
/* expect: 98 */
/* expect: 122 */
/* expect: 1 */
