/* A # that is not first on its line, in arguments over several lines, is a token. */
#define s(x) #x
#define f(x, y) s(x y)
s( a
  b # c
  x %: d) f(1 #
, ## 2)
