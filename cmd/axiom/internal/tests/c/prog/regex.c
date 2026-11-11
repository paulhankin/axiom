/* flags: --c-std=c99 */
/* Regular expressions two ways: a backtracking matcher working on the
   pattern text directly, and a compiler to a Thompson automaton simulated
   with state sets (bit vectors). The syntax: literals, '.', character
   classes [a-z] and [^...], '*', '+', '?', '|', groups, anchors '^' '$'
   (backtracking only) and the escape '\'. Both must agree on whether each
   subject contains a match and where the leftmost-longest one ends. */
#include "host.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* ---- the parse tree, shared by both engines ---- */

enum rk { R_CHAR, R_ANY, R_CLASS, R_CAT, R_ALT, R_STAR, R_PLUS, R_OPT, R_EMPTY };

struct rnode {
    enum rk kind;
    unsigned char ch;
    int a, b;
    uint32_t set[8];    /* 256-bit class */
};

static struct rnode rn[512];
static int nrn;
static const char *pp;
static int perr;

static int rnew(enum rk k) {
    if (nrn == 512) { perr = 1; return 0; }
    struct rnode *n = &rn[nrn];
    n->kind = k;
    n->a = n->b = -1;
    n->ch = 0;
    for (int i = 0; i < 8; i++) n->set[i] = 0;
    return nrn++;
}

static void set_bit(uint32_t *s, unsigned c) { s[c >> 5] |= 1u << (c & 31); }
static bool has_bit(const uint32_t *s, unsigned c) { return (s[c >> 5] >> (c & 31)) & 1u; }

static int parse_alt(void);

static int parse_atom(void) {
    char c = *pp;
    if (c == '(') {
        pp++;
        int e = parse_alt();
        if (*pp == ')') pp++; else perr = 1;
        return e;
    }
    if (c == '.') { pp++; return rnew(R_ANY); }
    if (c == '[') {
        pp++;
        int n = rnew(R_CLASS);
        bool neg = false;
        if (*pp == '^') { neg = true; pp++; }
        bool first = true;
        while (*pp && (*pp != ']' || first)) {
            unsigned lo = (unsigned char)*pp++;
            if (lo == '\\' && *pp) lo = (unsigned char)*pp++;
            unsigned hi = lo;
            if (*pp == '-' && pp[1] && pp[1] != ']') {
                pp++;
                hi = (unsigned char)*pp++;
            }
            for (unsigned x = lo; x <= hi; x++) set_bit(rn[n].set, x);
            first = false;
        }
        if (*pp == ']') pp++; else perr = 1;
        if (neg) for (int i = 0; i < 8; i++) rn[n].set[i] = ~rn[n].set[i];
        return n;
    }
    if (c == '\\' && pp[1]) {
        pp++;
        int n;
        switch (*pp) {
        case 'd':
            n = rnew(R_CLASS);
            for (unsigned x = '0'; x <= '9'; x++) set_bit(rn[n].set, x);
            break;
        case 'w':
            n = rnew(R_CLASS);
            for (unsigned x = 0; x < 256; x++)
                if ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z') || (x >= '0' && x <= '9') || x == '_')
                    set_bit(rn[n].set, x);
            break;
        case 's':
            n = rnew(R_CLASS);
            set_bit(rn[n].set, ' ');
            set_bit(rn[n].set, '\t');
            set_bit(rn[n].set, '\n');
            break;
        default:
            n = rnew(R_CHAR);
            rn[n].ch = (unsigned char)*pp;
            break;
        }
        pp++;
        return n;
    }
    if (c == 0 || c == '|' || c == ')' || c == '*' || c == '+' || c == '?') {
        if (c == '*' || c == '+' || c == '?') { perr = 1; pp++; }
        return rnew(R_EMPTY);
    }
    pp++;
    int n = rnew(R_CHAR);
    rn[n].ch = (unsigned char)c;
    return n;
}

static int parse_post(void) {
    int a = parse_atom();
    for (;;) {
        enum rk k;
        if (*pp == '*') k = R_STAR;
        else if (*pp == '+') k = R_PLUS;
        else if (*pp == '?') k = R_OPT;
        else return a;
        pp++;
        int n = rnew(k);
        rn[n].a = a;
        a = n;
    }
}

static int parse_cat(void) {
    if (*pp == 0 || *pp == '|' || *pp == ')') return rnew(R_EMPTY);
    int a = parse_post();
    while (*pp && *pp != '|' && *pp != ')') {
        int n = rnew(R_CAT);
        rn[n].a = a;
        rn[n].b = parse_post();
        a = n;
    }
    return a;
}

static int parse_alt(void) {
    int a = parse_cat();
    while (*pp == '|') {
        pp++;
        int n = rnew(R_ALT);
        rn[n].a = a;
        rn[n].b = parse_cat();
        a = n;
    }
    return a;
}

/* ---- backtracking over the tree, with continuations ---- */

struct cont {
    int node;
    const struct cont *next;
};

static const char *subject_start;
static long steps;
static int best_end;

static bool bt(int n, const char *s, const struct cont *k);

static bool bt_next(const char *s, const struct cont *k) {
    if (!k) {
        int e = (int)(s - subject_start);
        if (e > best_end) best_end = e;
        return false;   /* keep looking for a longer match */
    }
    return bt(k->node, s, k->next);
}

/* star: the loop node itself is the continuation of the body */
static bool bt_star(int n, const char *s, const struct cont *k, const char *last) {
    steps++;
    if (s != last) {   /* the body must consume something to loop */
        struct cont again = { -(n + 2), k };
        if (bt(rn[n].a, s, &again)) return true;
    }
    return bt_next(s, k);
}

static const char *star_pos[512];

static bool bt(int n, const char *s, const struct cont *k) {
    steps++;
    if (steps > 2000000) return false;
    if (n <= -2) {
        int m = -n - 2;
        const char *save = star_pos[m];
        star_pos[m] = s;
        bool r = bt_star(m, s, k, save);
        star_pos[m] = save;
        return r;
    }
    struct rnode *r = &rn[n];
    switch (r->kind) {
    case R_EMPTY: return bt_next(s, k);
    case R_CHAR: return *s && (unsigned char)*s == r->ch && bt_next(s + 1, k);
    case R_ANY: return *s && *s != '\n' && bt_next(s + 1, k);
    case R_CLASS: return *s && has_bit(r->set, (unsigned char)*s) && bt_next(s + 1, k);
    case R_CAT: {
        struct cont c = { r->b, k };
        return bt(r->a, s, &c);
    }
    case R_ALT: return bt(r->a, s, k) || bt(r->b, s, k);
    case R_OPT: return bt(r->a, s, k) || bt_next(s, k);
    case R_STAR: {
        const char *save = star_pos[n];
        star_pos[n] = s;
        bool res = bt_star(n, s, k, (const char *)0);
        star_pos[n] = save;
        return res;
    }
    case R_PLUS: {
        int star = n;   /* x+ = x x* : run the body then the star loop */
        struct cont c = { -(star + 2), k };
        const char *save = star_pos[n];
        star_pos[n] = (const char *)0;
        bool res = bt(r->a, s, &c);
        star_pos[n] = save;
        return res;
    }
    }
    return false;
}

/* longest match starting at s, or -1 */
static int bt_match_at(int root, const char *s) {
    best_end = -1;
    subject_start = s;
    bt(root, s, (const struct cont *)0);
    return best_end;
}

/* ---- Thompson automaton ---- */

enum sk { S_CHAR, S_ANY, S_CLASS, S_SPLIT, S_JMP, S_MATCH };

struct state {
    enum sk kind;
    unsigned char ch;
    int x, y;
    const uint32_t *set;
};

#define MAXS 1024
static struct state st[MAXS];
static int nst;

static int snew(enum sk k) {
    if (nst == MAXS) { perr = 1; return 0; }
    st[nst].kind = k;
    st[nst].x = st[nst].y = -1;
    st[nst].set = 0;
    return nst++;
}

/* compile node n so that it continues at `next`; returns its entry state */
static int comp(int n, int next) {
    struct rnode *r = &rn[n];
    int s;
    switch (r->kind) {
    case R_EMPTY: return next;
    case R_CHAR: s = snew(S_CHAR); st[s].ch = r->ch; st[s].x = next; return s;
    case R_ANY: s = snew(S_ANY); st[s].x = next; return s;
    case R_CLASS: s = snew(S_CLASS); st[s].set = r->set; st[s].x = next; return s;
    case R_CAT: return comp(r->a, comp(r->b, next));
    case R_ALT:
        s = snew(S_SPLIT);
        st[s].x = comp(r->a, next);
        st[s].y = comp(r->b, next);
        return s;
    case R_OPT:
        s = snew(S_SPLIT);
        st[s].x = comp(r->a, next);
        st[s].y = next;
        return s;
    case R_STAR:
        s = snew(S_SPLIT);
        st[s].x = comp(r->a, s);
        st[s].y = next;
        return s;
    case R_PLUS: {
        s = snew(S_SPLIT);
        int body = comp(r->a, s);
        st[s].x = body;
        st[s].y = next;
        return body;
    }
    }
    return next;
}

typedef struct { uint64_t w[MAXS / 64]; } sset;

static void sset_clear(sset *s) {
    for (int i = 0; i < MAXS / 64; i++) s->w[i] = 0;
}

static void add_state(sset *set, int s) {
    while (s >= 0) {
        if ((set->w[s >> 6] >> (s & 63)) & 1) return;
        set->w[s >> 6] |= (uint64_t)1 << (s & 63);
        if (st[s].kind == S_SPLIT) {
            add_state(set, st[s].x);
            s = st[s].y;
        } else if (st[s].kind == S_JMP) {
            s = st[s].x;
        } else {
            return;
        }
    }
}

static int nfa_steps;

static int nfa_match_at(int start, const char *s) {
    sset cur, nxt;
    sset_clear(&cur);
    add_state(&cur, start);
    int best = -1;
    for (int pos = 0;; pos++) {
        bool any = false;
        for (int i = 0; i < MAXS / 64; i++) {
            uint64_t w = cur.w[i];
            while (w) {
                int b = 0;
                while (!((w >> b) & 1)) b++;
                w &= w - 1;
                int q = i * 64 + b;
                if (st[q].kind == S_MATCH) best = pos;
                any = true;
            }
        }
        unsigned char c = (unsigned char)s[pos];
        if (!any || c == 0) return best;
        sset_clear(&nxt);
        for (int i = 0; i < MAXS / 64; i++) {
            uint64_t w = cur.w[i];
            while (w) {
                uint64_t low = w & (~w + 1);
                int b = 0;
                while (((uint64_t)1 << b) != low) b++;
                w ^= low;
                int q = i * 64 + b;
                nfa_steps++;
                switch (st[q].kind) {
                case S_CHAR: if (c == st[q].ch) add_state(&nxt, st[q].x); break;
                case S_ANY: if (c != '\n') add_state(&nxt, st[q].x); break;
                case S_CLASS: if (has_bit(st[q].set, c)) add_state(&nxt, st[q].x); break;
                default: break;
                }
            }
        }
        cur = nxt;
    }
}

/* ---- driver ---- */

struct result { int start, end; };

static struct result search(int root, int start, const char *s, bool nfa, bool anchored_begin) {
    struct result r = { -1, -1 };
    for (int i = 0;; i++) {
        int e = nfa ? nfa_match_at(start, s + i) : bt_match_at(root, s + i);
        if (e >= 0) { r.start = i; r.end = i + e; return r; }
        if (!s[i] || anchored_begin) return r;
    }
}

static const char *const patterns[] = {
    "abc", "a*b", "(a|b)*c", "a+b+", "colou?r", "[0-9]+", "x[^aeiou]y", "(ab|a)(bc|c)",
    "\\d+\\.\\d*", "\\w+@\\w+", "(a*)*b", "((a|b)(c|d))+", ".*z", "[a-c-]+", "h(e|ip)+o",
    "(x+x+)+y", "", "a?a?a?aaa", "[]a]+", "\\(\\)", "(((a)))", "(|a)b", "ab|cd|ef|gh",
};

static const char *const subjects[] = {
    "abc", "xxabcxx", "aaab", "bbbc", "abababc", "color", "colour", "colouur", "tel 555-0199 ext 12",
    "xby xay xzy", "abc", "abcd", "pi is 3.14159", "mail me@example now", "aaaaaaaaaaaaaaaaaaaaaaaab",
    "acbdadbc", "lazy fox z", "a-b-c-d", "hipipipeo", "heo", "xxxxxxxxxxxxxxxxxxxy", "xxxxxxxxxx", "",
    "aaaa", "]]aa]", "f()", "b", "zzgh", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaac",
};

int main(void) {
    int np = (int)(sizeof patterns / sizeof patterns[0]);
    int ns = (int)(sizeof subjects / sizeof subjects[0]);
    uint32_t h = 2166136261u;
    int disagreements = 0, matches = 0, parse_errors = 0;
    for (int p = 0; p < np; p++) {
        nrn = 0;
        perr = 0;
        pp = patterns[p];
        int root = parse_alt();
        if (*pp) perr = 1;
        if (perr) { parse_errors++; continue; }
        nst = 0;
        int m = snew(S_MATCH);
        int start = comp(root, m);
        int line_matches = 0;
        for (int s = 0; s < ns; s++) {
            steps = 0;
            struct result a = search(root, start, subjects[s], false, false);
            struct result b = search(root, start, subjects[s], true, false);
            if (a.start != b.start || a.end != b.end) disagreements++;
            if (a.start >= 0) { matches++; line_matches++; }
            h = (h ^ (uint32_t)(a.start + 1)) * 16777619u;
            h = (h ^ (uint32_t)(a.end + 1)) * 16777619u;
        }
        print_i64(p * 100 + line_matches);
    }
    /* selected results */
    {
        const char *pat = "(\\d+)-(\\d+)";
        nrn = 0; perr = 0; pp = pat;
        int root = parse_alt();
        nst = 0;
        int start = comp(root, snew(S_MATCH));
        struct result r = search(root, start, "tel 555-0199 ext 12", true, false);
        print_i64(r.start);
        print_i64(r.end);
        print_str("tel 555-0199 ext 12" + r.start, (size_t)(r.end - r.start));
    }
    print_i64(matches);
    print_i64(disagreements);
    print_i64(parse_errors);
    print_i64(nfa_steps);
    print_u64(h);
    return 0;
}

/* expect: 5 */
/* expect: 112 */
/* expect: 212 */
/* expect: 307 */
/* expect: 402 */
/* expect: 502 */
/* expect: 602 */
/* expect: 707 */
/* expect: 801 */
/* expect: 901 */
/* expect: 1012 */
/* expect: 1108 */
/* expect: 1203 */
/* expect: 1321 */
/* expect: 1402 */
/* expect: 1501 */
/* expect: 1629 */
/* expect: 1704 */
/* expect: 1815 */
/* expect: 1901 */
/* expect: 2015 */
/* expect: 2112 */
/* expect: 2208 */
/* expect: 4 */
/* expect: 12 */
/* expect: 555-0199 */
/* expect: 182 */
/* expect: 0 */
/* expect: 0 */
/* expect: 31652 */
/* expect: 3918682588 */
