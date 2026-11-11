/* # of a lone backslash makes no string literal. */
/* expect-error: C1066 */
#define s(x) #x
s(\)
