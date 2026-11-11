/* A small program: a generic sort through a comparison function, a
   binary search, a hash table of strings with chaining, and a recursive
   evaluator over a tree of structures. */
#include "host.h"

typedef int (*cmp_fn)(const void *, const void *);

static void swap_bytes(char *a, char *b, size_t n) {
    while (n--) {
        char t = *a;
        *a++ = *b;
        *b++ = t;
    }
}

static void sort(void *base, size_t n, size_t size, cmp_fn cmp) {
    char *b = base;
    size_t i, j;
    for (i = 1; i < n; i++)
        for (j = i; j > 0 && cmp(b + (j - 1) * size, b + j * size) > 0; j--)
            swap_bytes(b + (j - 1) * size, b + j * size, size);
}

static int cmp_int(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

struct person { const char *name; int age; };

static int cmp_age(const void *a, const void *b) {
    const struct person *p = a, *q = b;
    return p->age - q->age;
}

static int find(const int *a, int n, int key) {
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (a[mid] == key) return mid;
        if (a[mid] < key) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

#define NB 7
struct entry { const char *key; int value; struct entry *next; };
static struct entry pool[32];
static int used;
static struct entry *buckets[NB];

static unsigned hash(const char *s) {
    unsigned h = 5381;
    while (*s) h = h * 33 + (unsigned char)*s++;
    return h;
}

static int streq(const char *a, const char *b) {
    while (*a && *a == *b) a++, b++;
    return *a == *b;
}

static void put(const char *k, int v) {
    unsigned h = hash(k) % NB;
    struct entry *e;
    for (e = buckets[h]; e; e = e->next)
        if (streq(e->key, k)) { e->value = v; return; }
    e = &pool[used++];
    e->key = k;
    e->value = v;
    e->next = buckets[h];
    buckets[h] = e;
}

static int get(const char *k) {
    struct entry *e;
    for (e = buckets[hash(k) % NB]; e; e = e->next)
        if (streq(e->key, k)) return e->value;
    return -1;
}

enum op { NUM, ADD, MUL, NEG };
struct node { enum op op; int val; struct node *l, *r; };

static int eval(const struct node *n) {
    switch (n->op) {
    case NUM: return n->val;
    case ADD: return eval(n->l) + eval(n->r);
    case MUL: return eval(n->l) * eval(n->r);
    case NEG: return -eval(n->l);
    }
    return 0;
}

int main(void) {
    int a[] = { 5, 3, 9, 1, 7, 2, 8, 6, 4, 0 };
    struct person ps[] = { { "ann", 31 }, { "bob", 25 }, { "cy", 40 }, { "di", 19 } };
    int i;
    struct node n1 = { NUM, 6, 0, 0 }, n2 = { NUM, 7, 0, 0 }, n3 = { MUL, 0, &n1, &n2 };
    struct node n4 = { NUM, 2, 0, 0 }, n5 = { NEG, 0, &n4, 0 }, n6 = { ADD, 0, &n3, &n5 };
    sort(a, 10, sizeof a[0], cmp_int);
    for (i = 0; i < 10; i++) print_i64(a[i]);
    print_i64(find(a, 10, 7));
    print_i64(find(a, 10, 11));
    sort(ps, 4, sizeof ps[0], cmp_age);
    for (i = 0; i < 4; i++) print_str(ps[i].name, ps[i].name[2] ? 3 : 2);
    put("alpha", 1);
    put("beta", 2);
    put("gamma", 3);
    put("beta", 20);
    print_i64(get("alpha") + get("beta") + get("gamma") + get("delta"));
    print_i64(used);
    print_i64(eval(&n6));
    return 0;
}

/* expect: 0 */
/* expect: 1 */
/* expect: 2 */
/* expect: 3 */
/* expect: 4 */
/* expect: 5 */
/* expect: 6 */
/* expect: 7 */
/* expect: 8 */
/* expect: 9 */
/* expect: 7 */
/* expect: -1 */
/* expect: di */
/* expect: bob */
/* expect: ann */
/* expect: cy */
/* expect: 23 */
/* expect: 3 */
/* expect: 40 */
