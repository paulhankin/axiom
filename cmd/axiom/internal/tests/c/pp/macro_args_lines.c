/* Arguments that span lines; new-lines in them are white space. */
#define f(a, b) <a|b>
#define s(x) #x
f(1
,
2)
f(
  (a,
   b), c
)
s(one
  two
three) after
next line
