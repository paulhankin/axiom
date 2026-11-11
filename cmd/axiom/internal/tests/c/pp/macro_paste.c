/* ##: identifiers, numbers, punctuators, with empty arguments; the result is rescanned. */
/* flags: --c-std=c99 */
#define cat(a, b) a ## b
#define xcat(a, b) cat(a, b)
#define s(x) #x
#define xs(x) s(x)
cat(x, y) cat(1, 2) cat(+, +) cat(-, =) cat(<, <=) cat(., 5) xs(cat(1, .5e)) xs(cat(1e, +)) cat(0x, 1p-3)
cat(, x) cat(x, ) cat(,)
#define xy done
cat(x, y) xcat(x, y) xcat(xcat(1, 2), 3)
#define cat3(a, b, c) a ## b ## c
cat3(a, b, c) cat3(, , c) cat3(a, , ) cat3(1, ., 2)
#define obj a ## b
#define ab paste
obj
#define L_(x) L ## x
L_("wide") L_('c')
#define hh %: %: ## %:
xs(hh)
#define dig(x) %:x
dig(y)
