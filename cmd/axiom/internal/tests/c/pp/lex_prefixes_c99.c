/* Before C11, u, U and u8 are identifiers in front of a literal. */
/* flags: --c-std=c99 */
#define u
#define U
#define u8
u"a" U"b" u8"c" L"d" u'e' L'f'
