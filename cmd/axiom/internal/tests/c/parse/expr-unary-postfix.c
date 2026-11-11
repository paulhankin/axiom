/* Unary and postfix operators. */
struct s { int m; struct s *p; };
int f(int a, int *p, struct s v) {
    a = -a + +a - ~a + !a;
    a = *p + *&a;
    ++a;
    --a;
    a++;
    a--;
    a = p[0] + v.m + v.p->m + v.p->p[0].m;
    return f(a, p, v) + (a, a);
}
