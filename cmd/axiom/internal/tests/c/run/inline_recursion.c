/* Inlining in recursion: small functions in cycles of calls, whose copies
   bring calls along that lead back to them (several calls to itself,
   mutual recursion with fan-out 2 and 3, a cycle of five functions with
   fan-out 3, a continuation step between a matcher and itself). The
   expansion must stay bounded (the build would not finish otherwise) and
   the results must be those of the calls. */
#include "host.h"

static long six(int n) {
    if (n <= 0) return 1;
    return six(n - 1) + six(n - 2) + six(n - 3) + six(n - 4) + six(n - 5) + six(n - 6);
}

static long mb(int n);
static long ma(int n) { return n <= 0 ? 1 : mb(n - 1) + mb(n - 2); }
static long mb(int n) { return n <= 0 ? 2 : ma(n - 1) + ma(n - 2) + 1; }

static long pb(int n);
static long pa(int n) { return n <= 0 ? 1 : pb(n - 1) + pb(n - 2) + pb(n - 3); }
static long pb(int n) { return n <= 0 ? 3 : 2 * pa(n - 1) + pa(n - 2) + pa(n - 3); }

static long c2(int n), c3(int n), c4(int n), c5(int n);
static long c1(int n) { return n <= 0 ? 1 : c2(n - 1) + c2(n - 2) + c2(n - 3); }
static long c2(int n) { return n <= 0 ? 2 : c3(n - 1) + c3(n - 2) + c3(n - 3); }
static long c3(int n) { return n <= 0 ? 3 : c4(n - 1) + c4(n - 2) + c4(n - 3); }
static long c4(int n) { return n <= 0 ? 4 : c5(n - 1) + c5(n - 2) + c5(n - 3); }
static long c5(int n) { return n <= 0 ? 5 : c1(n - 1) + c1(n - 2) + c1(n - 3); }

/* each of the five also called from outside its cycle */
static long out5(int n) { return c1(n) + c2(n) + c3(n) + c4(n) + c5(n); }

struct cont { int node; const struct cont *next; };
static int found;
static int walk(int n, const char *s, const struct cont *k);
static int step(const char *s, const struct cont *k) {
    if (!k) {
        found += 1;
        return 0;
    }
    return walk(k->node, s, k->next);
}
static int walk(int n, const char *s, const struct cont *k) {
    if (n == 0) return step(s, k);
    if (*s == 'a') {
        struct cont c = { n - 1, k };
        if (walk(0, s + 1, &c)) return 1;
    }
    return step(s, k);
}

int main(void) {
    print_i64(six(12));
    print_i64(ma(14));
    print_i64(pa(10));
    print_i64(c1(10));
    print_i64(out5(8));
    walk(4, "aaaaaa", 0);
    print_i64(found);
    return 0;
}
/* expect: 9841 */
/* expect: 1973 */
/* expect: 5217 */
/* expect: 1720 */
/* expect: 2895 */
/* expect: 5 */
