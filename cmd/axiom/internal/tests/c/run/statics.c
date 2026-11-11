/* Objects with static storage duration: initializers that are constant
   expressions and address constants, tentative definitions, extern,
   internal linkage, block-scope statics, string literals. */
#include "host.h"

int tent;
int tent;
int tent2[];
static int st = 3 * 4 + 1;
extern int later;
int later = 77;
int arr[5] = { 10, 20, 30, 40, 50 };
int *pa = &arr[2];
int *pb = arr + 4;
int *pnull = 0;
char *str = "static string";
const char *strs[] = { "zero", "one", "two" };
char *mid = "abcdef" + 3;
struct pt { int x, y; } pts[2] = { { 1, 2 }, { 3, 4 } };
int *py = &pts[1].y;
struct pt *pp = &pts[1];
int (*fn)(void);
static int seven(void) { return 7; }
int (*fns[2])(void) = { seven, 0 };
double dv = 1.5 * 2;
float fv = 1 / 3.0;
unsigned long ul = -1;
char c = 300;
short sh = 70000;
_Bool bo = 42;
long long ll = 1LL << 50;
int ilist[] = { 1, 2, 3, };
struct { char name[8]; int id; } rec = { "bob", 9 };
struct { int a; char *s; } recs[] = { { 1, "x" }, { 2, "yy" } };

static int counter(void) {
    static int n;
    static int m = 100;
    n++;
    m--;
    return n * 1000 + m;
}

static int *addr(void) {
    static int v = 5;
    return &v;
}

int main(void) {
    print_i64(tent);
    tent = 4;
    print_i64(tent);
    print_i64(tent2[0]);
    print_i64(st);
    print_i64(later);
    print_i64(*pa);
    print_i64(*pb);
    print_i64(pnull == 0);
    print_str(str, 13);
    print_str(strs[2], 3);
    print_i64(*mid);
    print_i64(*py);
    print_i64(pp->x);
    print_i64(fn == 0);
    print_i64(fns[0]());
    print_i64(fns[1] == 0);
    print_f64(dv);
    print_f64(fv);
    print_i64(ul == (unsigned long)-1);
    print_i64(c);
    print_i64(sh);
    print_i64(bo);
    print_i64(ll);
    print_i64(sizeof ilist / sizeof *ilist);
    print_str(rec.name, 3);
    print_i64(rec.name[5] + rec.id);
    print_i64(recs[1].s[1]);
    print_i64(counter());
    print_i64(counter());
    print_i64(counter());
    *addr() += 10;
    print_i64(*addr());
    {
        extern int arr[5];
        print_i64(arr[1]);
    }
    {
        static const char hex[] = "0123456789abcdef";
        print_i64(hex[15]);
        print_i64(sizeof hex);
    }
    print_i64("abc"[1]);
    print_i64(sizeof "abc");
    return 0;
}

/* expect: 0 */
/* expect: 4 */
/* expect: 0 */
/* expect: 13 */
/* expect: 77 */
/* expect: 30 */
/* expect: 50 */
/* expect: 1 */
/* expect: static string */
/* expect: two */
/* expect: 100 */
/* expect: 4 */
/* expect: 3 */
/* expect: 1 */
/* expect: 7 */
/* expect: 1 */
/* expect: 3 */
/* expect: 0.3333333432674408 */
/* expect: 1 */
/* expect: 44 */
/* expect: 4464 */
/* expect: 1 */
/* expect: 1125899906842624 */
/* expect: 3 */
/* expect: bob */
/* expect: 9 */
/* expect: 121 */
/* expect: 1099 */
/* expect: 2098 */
/* expect: 3097 */
/* expect: 15 */
/* expect: 20 */
/* expect: 102 */
/* expect: 17 */
/* expect: 98 */
/* expect: 4 */
