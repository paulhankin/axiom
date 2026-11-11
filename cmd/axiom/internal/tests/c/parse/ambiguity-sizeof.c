/* sizeof of a type name, of a parenthesized expression, of an expression,
   and of a compound literal. */
typedef int T;
struct s { int m; };
int f(int x) {
    unsigned long a = sizeof(T);
    unsigned long b = sizeof(x);
    unsigned long c = sizeof x;
    unsigned long d = sizeof(struct s){1}.m;
    unsigned long e = sizeof(T) * 2;
    return (int)(a + b + c + d + e);
}
