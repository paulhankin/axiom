/* Typedef names as member names, labels, after '.' and '->', and as tags. */
typedef int T;
struct T { T T; int U; } s;
int f(struct T *p) {
    T:
    p->T = s.T;
    if (p->T)
        goto T;
    return p->U;
}
