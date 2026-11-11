/* flags: --c-std=c11 */
/* A small Lisp: cells in a fixed arena with a mark-and-sweep collector,
   values as tagged indices, a reader, an evaluator with closures and
   environments as association lists, and a set of programs that make the
   collector run many times. Every temporary the evaluator holds across an
   allocation is registered in a root stack (pointers to its variables). */
#include "host.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef int32_t val;   /* < 0: a small integer -(n+1)... see below */

enum kind { K_FREE, K_CONS, K_SYM, K_NUM, K_PRIM, K_CLOS };

struct cell {
    unsigned char kind;
    unsigned char mark;
    union {
        struct { val car, cdr; } pair;
        struct { int name; } sym;
        int64_t num;
        struct { int id; } prim;
        struct { val params, body, env; } clos;
    } u;
};

#define NCELLS 6000
static struct cell heap[NCELLS];
static val free_list;
static int nfree;
static int collections;
static int64_t allocated_total;

#define NIL 0
static val T_SYM, QUOTE, IF, LAMBDA, DEFINE, LET, SETQ, BEGIN, COND, ELSE;

/* The root stack: addresses of the variables that hold live values. */
static val *roots[20000];
static int nroots;
#define PUSH(v) (roots[nroots++] = &(v))
#define POP(n) (nroots -= (n))

static val global_env;

static char symtab[128][12];
static int nsyms;
static val symcells[128];

static void mark(val v) {
    while (v > 0 && !heap[v].mark) {
        heap[v].mark = 1;
        switch (heap[v].kind) {
        case K_CONS:
            mark(heap[v].u.pair.car);
            v = heap[v].u.pair.cdr;
            break;
        case K_CLOS:
            mark(heap[v].u.clos.params);
            mark(heap[v].u.clos.body);
            v = heap[v].u.clos.env;
            break;
        default:
            return;
        }
    }
}

static void collect(void) {
    collections++;
    for (int i = 0; i < nroots; i++) mark(*roots[i]);
    for (int i = 0; i < nsyms; i++) mark(symcells[i]);
    mark(global_env);
    free_list = NIL;
    nfree = 0;
    for (int i = NCELLS - 1; i > 0; i--) {
        if (heap[i].mark) {
            heap[i].mark = 0;
        } else {
            heap[i].kind = K_FREE;
            heap[i].u.pair.cdr = free_list;
            free_list = i;
            nfree++;
        }
    }
}

static int out_of_memory;

static val alloc(enum kind k) {
    if (free_list == NIL) collect();
    if (free_list == NIL) { out_of_memory = 1; return NIL; }
    val v = free_list;
    free_list = heap[v].u.pair.cdr;
    nfree--;
    allocated_total++;
    heap[v].kind = (unsigned char)k;
    heap[v].mark = 0;
    heap[v].u.pair.car = heap[v].u.pair.cdr = NIL;
    return v;
}

static val cons(val a, val d) {
    PUSH(a);
    PUSH(d);
    val c = alloc(K_CONS);
    POP(2);
    heap[c].u.pair.car = a;
    heap[c].u.pair.cdr = d;
    return c;
}

static val num(int64_t n) {
    val c = alloc(K_NUM);
    heap[c].u.num = n;
    return c;
}

static val car(val v) { return v > 0 && heap[v].kind == K_CONS ? heap[v].u.pair.car : NIL; }
static val cdr(val v) { return v > 0 && heap[v].kind == K_CONS ? heap[v].u.pair.cdr : NIL; }
static bool is(val v, enum kind k) { return v > 0 && heap[v].kind == k; }
static int64_t numval(val v) { return is(v, K_NUM) ? heap[v].u.num : 0; }

static val intern(const char *s, int n) {
    if (n > 11) n = 11;
    for (int i = 0; i < nsyms; i++) {
        int j = 0;
        while (j < n && symtab[i][j] == s[j]) j++;
        if (j == n && symtab[i][n] == 0) return symcells[i];
    }
    for (int j = 0; j < n; j++) symtab[nsyms][j] = s[j];
    symtab[nsyms][n] = 0;
    val c = alloc(K_SYM);
    heap[c].u.sym.name = nsyms;
    symcells[nsyms] = c;
    nsyms++;
    return c;
}

static val sym(const char *s) {
    int n = 0;
    while (s[n]) n++;
    return intern(s, n);
}

/* The reader. */
static const char *rp;

static void skip_space(void) {
    for (;;) {
        while (*rp == ' ' || *rp == '\n' || *rp == '\t') rp++;
        if (*rp == ';') {
            while (*rp && *rp != '\n') rp++;
            continue;
        }
        return;
    }
}

static val read_expr(void);

static val read_list(void) {
    skip_space();
    if (*rp == ')') { rp++; return NIL; }
    if (*rp == 0) return NIL;
    val head = read_expr();
    PUSH(head);
    val tail = read_list();
    POP(1);
    return cons(head, tail);
}

static val read_expr(void) {
    skip_space();
    char c = *rp;
    if (c == '(') { rp++; return read_list(); }
    if (c == '\'') {
        rp++;
        val e = read_expr();
        PUSH(e);
        val l = cons(e, NIL);
        POP(1);
        return cons(QUOTE, l);
    }
    if ((c >= '0' && c <= '9') || (c == '-' && rp[1] >= '0' && rp[1] <= '9')) {
        bool neg = c == '-';
        if (neg) rp++;
        int64_t n = 0;
        while (*rp >= '0' && *rp <= '9') n = n * 10 + (*rp++ - '0');
        return num(neg ? -n : n);
    }
    const char *s = rp;
    while (*rp && *rp != ' ' && *rp != '(' && *rp != ')' && *rp != '\n' && *rp != '\t') rp++;
    return intern(s, (int)(rp - s));
}

/* Environments: a list of frames, each a list of (symbol . value). */
static val lookup_cell(val env, val s) {
    for (; env != NIL; env = cdr(env))
        for (val f = car(env); f != NIL; f = cdr(f))
            if (car(car(f)) == s) return car(f);
    return NIL;
}

static void define(val env, val s, val v) {
    PUSH(env);
    PUSH(s);
    PUSH(v);
    val b = cons(s, v);
    PUSH(b);
    val f = cons(b, car(env));
    heap[env].u.pair.car = f;
    POP(4);
}

enum prim_id { P_ADD, P_SUB, P_MUL, P_DIV, P_MOD, P_LT, P_EQ, P_CONS, P_CAR, P_CDR, P_NULL, P_LIST, P_PAIR, P_NUMP };
static const char *const prim_names[] = {
    "+", "-", "*", "/", "mod", "<", "=", "cons", "car", "cdr", "null?", "list", "pair?", "number?"
};

static val apply_prim(int id, val args) {
    int64_t a = numval(car(args)), b = numval(car(cdr(args)));
    switch ((enum prim_id)id) {
    case P_ADD: {
        int64_t s = 0;
        for (val x = args; x != NIL; x = cdr(x)) s += numval(car(x));
        return num(s);
    }
    case P_SUB: return cdr(args) == NIL ? num(-a) : num(a - b);
    case P_MUL: {
        int64_t s = 1;
        for (val x = args; x != NIL; x = cdr(x)) s *= numval(car(x));
        return num(s);
    }
    case P_DIV: return num(b ? a / b : 0);
    case P_MOD: return num(b ? a % b : 0);
    case P_LT: return a < b ? T_SYM : NIL;
    case P_EQ: {
        val x = car(args), y = car(cdr(args));
        if (is(x, K_NUM) && is(y, K_NUM)) return a == b ? T_SYM : NIL;
        return x == y ? T_SYM : NIL;
    }
    case P_CONS: return cons(car(args), car(cdr(args)));
    case P_CAR: return car(car(args));
    case P_CDR: return cdr(car(args));
    case P_NULL: return car(args) == NIL ? T_SYM : NIL;
    case P_LIST: return args;
    case P_PAIR: return is(car(args), K_CONS) ? T_SYM : NIL;
    case P_NUMP: return is(car(args), K_NUM) ? T_SYM : NIL;
    }
    return NIL;
}

static int depth, max_depth;
static val eval(val x, val env);

static val evlis(val l, val env) {
    if (l == NIL) return NIL;
    PUSH(l);
    PUSH(env);
    val h = eval(car(l), env);
    PUSH(h);
    val t = evlis(cdr(l), env);
    val r = cons(h, t);
    POP(3);
    return r;
}

static val bind(val params, val args) {
    val frame = NIL;
    PUSH(params);
    PUSH(args);
    PUSH(frame);
    for (; params != NIL; params = cdr(params), args = cdr(args)) {
        val b = cons(car(params), car(args));
        frame = cons(b, frame);
    }
    POP(3);
    return frame;
}

static val eval(val x, val env) {
    val r = NIL, f = NIL, args = NIL;
    PUSH(x);
    PUSH(env);
    PUSH(f);
    PUSH(args);
    if (++depth > max_depth) max_depth = depth;
tail:
    if (x == NIL || out_of_memory) { r = NIL; goto done; }
    if (is(x, K_NUM) || is(x, K_PRIM) || is(x, K_CLOS)) { r = x; goto done; }
    if (is(x, K_SYM)) {
        if (x == T_SYM) { r = x; goto done; }
        r = cdr(lookup_cell(env, x));
        goto done;
    }
    {
        val op = car(x);
        if (op == QUOTE) { r = car(cdr(x)); goto done; }
        if (op == IF) {
            val c = eval(car(cdr(x)), env);
            x = c != NIL ? car(cdr(cdr(x))) : car(cdr(cdr(cdr(x))));
            goto tail;
        }
        if (op == COND) {
            for (val cl = cdr(x); cl != NIL; cl = cdr(cl)) {
                val test = car(car(cl));
                if (test == ELSE || eval(test, env) != NIL) {
                    x = car(cdr(car(cl)));
                    goto tail;
                }
            }
            r = NIL;
            goto done;
        }
        if (op == LAMBDA) {
            r = alloc(K_CLOS);
            heap[r].u.clos.params = car(cdr(x));
            heap[r].u.clos.body = cdr(cdr(x));
            heap[r].u.clos.env = env;
            goto done;
        }
        if (op == DEFINE) {
            val v = eval(car(cdr(cdr(x))), env);
            define(global_env, car(cdr(x)), v);
            r = car(cdr(x));
            goto done;
        }
        if (op == SETQ) {
            val v = eval(car(cdr(cdr(x))), env);
            val b = lookup_cell(env, car(cdr(x)));
            if (b != NIL) heap[b].u.pair.cdr = v;
            r = v;
            goto done;
        }
        if (op == BEGIN) {
            val body = cdr(x);
            if (body == NIL) { r = NIL; goto done; }
            while (cdr(body) != NIL) {
                eval(car(body), env);
                body = cdr(body);
            }
            x = car(body);
            goto tail;
        }
        if (op == LET) {
            /* (let ((a 1) (b 2)) body...) */
            val frame = NIL;
            PUSH(frame);
            for (val bs = car(cdr(x)); bs != NIL; bs = cdr(bs)) {
                val v = eval(car(cdr(car(bs))), env);
                PUSH(v);
                val b = cons(car(car(bs)), v);
                POP(1);
                frame = cons(b, frame);
            }
            env = cons(frame, env);
            POP(1);
            val body = cdr(cdr(x));
            while (cdr(body) != NIL) {
                eval(car(body), env);
                body = cdr(body);
            }
            x = car(body);
            goto tail;
        }
        f = eval(op, env);
        args = evlis(cdr(x), env);
        if (is(f, K_PRIM)) {
            r = apply_prim(heap[f].u.prim.id, args);
            goto done;
        }
        if (is(f, K_CLOS)) {
            val frame = bind(heap[f].u.clos.params, args);
            env = cons(frame, heap[f].u.clos.env);
            val body = heap[f].u.clos.body;
            while (cdr(body) != NIL) {
                eval(car(body), env);
                body = cdr(body);
            }
            x = car(body);
            goto tail;
        }
        r = NIL;
    }
done:
    depth--;
    POP(4);
    return r;
}

/* Printing into a buffer. */
static char outbuf[512];
static int outn;

static void put(char c) {
    if (outn < (int)sizeof outbuf - 1) outbuf[outn++] = c;
}

static void put_num(int64_t n) {
    char tmp[24];
    int k = 0;
    uint64_t u = n < 0 ? 0 - (uint64_t)n : (uint64_t)n;
    do { tmp[k++] = (char)('0' + u % 10); u /= 10; } while (u);
    if (n < 0) put('-');
    while (k) put(tmp[--k]);
}

static void print_val(val v) {
    if (v == NIL) { put('('); put(')'); return; }
    switch (heap[v].kind) {
    case K_NUM: put_num(heap[v].u.num); break;
    case K_SYM:
        for (const char *s = symtab[heap[v].u.sym.name]; *s; s++) put(*s);
        break;
    case K_PRIM: put('#'); put('p'); break;
    case K_CLOS: put('#'); put('f'); break;
    case K_CONS:
        put('(');
        for (;;) {
            print_val(car(v));
            v = cdr(v);
            if (v == NIL) break;
            if (!is(v, K_CONS)) {
                put(' '); put('.'); put(' ');
                print_val(v);
                break;
            }
            put(' ');
        }
        put(')');
        break;
    default: put('?'); break;
    }
}

static void show(val v) {
    outn = 0;
    print_val(v);
    print_str(outbuf, (size_t)outn);
}

static const char program[] =
    "(define fact (lambda (n) (if (< n 2) 1 (* n (fact (- n 1))))))\n"
    "(fact 20)\n"
    "(define fib (lambda (n) (if (< n 2) n (+ (fib (- n 1)) (fib (- n 2))))))\n"
    "(fib 18)\n"
    "(define map (lambda (f l) (if (null? l) '() (cons (f (car l)) (map f (cdr l))))))\n"
    "(define range (lambda (a b) (if (< a b) (cons a (range (+ a 1) b)) '())))\n"
    "(map (lambda (x) (* x x)) (range 0 12))\n"
    "(define rev (lambda (l acc) (if (null? l) acc (rev (cdr l) (cons (car l) acc)))))\n"
    "(rev (range 0 10) '())\n"
    "(define len (lambda (l) (if (null? l) 0 (+ 1 (len (cdr l))))))\n"
    "(len (range 0 300))\n"
    "(define make-counter (lambda () (let ((n 0)) (lambda () (set! n (+ n 1)) n))))\n"
    "(define c1 (make-counter))\n"
    "(define c2 (make-counter))\n"
    "(c1) (c1) (c2)\n"
    "(list (c1) (c2) (c1))\n"
    "(define foldl (lambda (f acc l) (if (null? l) acc (foldl f (f acc (car l)) (cdr l)))))\n"
    "(foldl + 0 (range 1 101))\n"
    "(define filter (lambda (p l) (cond ((null? l) '()) ((p (car l)) (cons (car l) (filter p (cdr l)))) (else (filter p (cdr l))))))\n"
    "(filter (lambda (x) (= 0 (mod x 3))) (range 0 40))\n"
    "(define append (lambda (a b) (if (null? a) b (cons (car a) (append (cdr a) b)))))\n"
    "(define qsort (lambda (l) (if (null? l) '() (let ((p (car l)) (r (cdr l)))"
    " (append (qsort (filter (lambda (x) (< x p)) r)) (cons p (qsort (filter (lambda (x) (< p (+ x 1))) r))))))))\n"
    "(qsort '(31 4 15 92 65 35 89 79 32 38 46 26 43 38 32 79 50 28 84 19 71 69 39 93 75 10 58 20 97 49))\n"
    "(define loop (lambda (i acc) (if (= i 0) acc (loop (- i 1) (+ acc (len (range 0 (mod i 17))))))))\n"
    "(loop 400 0)\n"
    "(define tree (lambda (d) (if (= d 0) 1 (cons (tree (- d 1)) (tree (- d 1))))))\n"
    "(define count (lambda (tr) (if (pair? tr) (+ (count (car tr)) (count (cdr tr))) 1)))\n"
    "(count (tree 9))\n"
    "(let ((a 3) (b 4)) (begin (+ (* a a) (* b b))))\n"
    "'(a (b . c) d)\n"
    "(define compose (lambda (f g) (lambda (x) (f (g x)))))\n"
    "((compose (lambda (x) (* x 2)) (lambda (x) (+ x 10))) 5)\n"
    "(define ack (lambda (m n) (cond ((= m 0) (+ n 1)) ((= n 0) (ack (- m 1) 1)) (else (ack (- m 1) (ack m (- n 1)))))))\n"
    "(ack 2 3)\n";

int main(void) {
    free_list = NIL;
    for (int i = NCELLS - 1; i > 0; i--) {
        heap[i].kind = K_FREE;
        heap[i].u.pair.cdr = free_list;
        free_list = i;
        nfree++;
    }
    global_env = NIL;
    PUSH(global_env);
    global_env = cons(NIL, NIL);
    T_SYM = sym("t");
    QUOTE = sym("quote");
    IF = sym("if");
    LAMBDA = sym("lambda");
    DEFINE = sym("define");
    LET = sym("let");
    SETQ = sym("set!");
    BEGIN = sym("begin");
    COND = sym("cond");
    ELSE = sym("else");
    for (int i = 0; i < (int)(sizeof prim_names / sizeof prim_names[0]); i++) {
        val p = alloc(K_PRIM);
        heap[p].u.prim.id = i;
        PUSH(p);
        define(global_env, sym(prim_names[i]), p);
        POP(1);
    }
    rp = program;
    int n = 0;
    uint64_t h = 1469598103934665603ull;
    for (;;) {
        skip_space();
        if (*rp == 0) break;
        val e = read_expr();
        PUSH(e);
        val r = eval(e, global_env);
        POP(1);
        outn = 0;
        print_val(r);
        for (int i = 0; i < outn; i++) h = (h ^ (unsigned char)outbuf[i]) * 1099511628211ull;
        if (!is(e, K_CONS) || car(e) != DEFINE) show(r);
        n++;
    }
    print_i64(n);
    print_i64(nroots);
    print_i64(out_of_memory);
    print_i64(collections > 10);
    print_i64(allocated_total);
    print_i64(collections);
    print_i64(max_depth);
    print_i64(nsyms);
    print_u64(h);
    return 0;
}

/* expect: 2432902008176640000 */
/* expect: 2584 */
/* expect: (0 1 4 9 16 25 36 49 64 81 100 121) */
/* expect: (9 8 7 6 5 4 3 2 1 0) */
/* expect: 300 */
/* expect: 1 */
/* expect: 2 */
/* expect: 1 */
/* expect: (3 2 4) */
/* expect: 5050 */
/* expect: (0 3 6 9 12 15 18 21 24 27 30 33 36 39) */
/* expect: (4 10 15 19 20 26 28 31 32 32 35 38 38 39 43 46 49 50 58 65 69 71 75 79 79 84 89 92 93 97) */
/* expect: 3173 */
/* expect: 512 */
/* expect: 25 */
/* expect: (a (b . c) d) */
/* expect: 30 */
/* expect: 9 */
/* expect: 36 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 216558 */
/* expect: 38 */
/* expect: 304 */
/* expect: 58 */
/* expect: 8376353462479768612 */
