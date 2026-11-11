/* Maximal munch, and a space wherever re-lexing would join tokens. */
/* flags: --c-std=c23 */
#define e
#define cat(a, b) a b
x+++++y a-->b x...y x..y a<<=b a>>=b a->b a::b
- e - + e + - e > . e . e . . e 1 x e y 1 e .2 a e = b % e : < e : / e / / e *
cat(-, -) cat(<, <) cat(., 3)
