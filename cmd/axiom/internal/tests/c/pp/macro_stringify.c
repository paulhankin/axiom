/* #: spacing, quotes and backslashes in literals, empty arguments, a string's prefix. */
/* flags: --c-std=c11 */
#define s(x) #x
#define xs(x) s(x)
s(  a   b  ) s(a/**/b) s(a  /* c */  b) s() s( )
s("x\n" '\'' '"' "\"") s(\n) s(L"a" u8"b" u"c" U'd')
s((a , b)) s(  +  -  ) s(@) s(a +
   b)
xs(__LINE__) xs(s(x)) s(s(x))
#define v(...) #__VA_ARGS__
v() v(a,b) v( a , b ) v(,) v(a,)
