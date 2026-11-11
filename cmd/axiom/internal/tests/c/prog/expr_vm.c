/* flags: --c-std=c99 */
/* An expression language: a tokenizer, a recursive-descent parser building a
   tree in a node pool, a constant folder, a compiler to a stack machine's
   code and its interpreter. The tree is also evaluated directly; both must
   agree for every program. Variables, assignment, the conditional operator,
   short-circuit logic, comparisons and calls of a few built-in functions. */
#include "host.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

enum tok {
    T_END, T_NUM, T_NAME, T_PLUS, T_MINUS, T_STAR, T_SLASH, T_PERCENT, T_LP, T_RP,
    T_ASSIGN, T_EQ, T_NE, T_LT, T_LE, T_GT, T_GE, T_AND, T_OR, T_NOT, T_QUEST,
    T_COLON, T_SEMI, T_COMMA, T_SHL, T_SHR, T_AMP, T_PIPE, T_CARET, T_TILDE, T_BAD
};

struct token {
    enum tok kind;
    int64_t num;
    int name;      /* index into the name table */
    int pos;
};

#define MAX_TOKENS 2048
#define MAX_NAMES 64
#define NAME_LEN 16

static struct token toks[MAX_TOKENS];
static int ntoks;
static char names[MAX_NAMES][NAME_LEN];
static int nnames;

static int intern(const char *s, int n) {
    if (n >= NAME_LEN) n = NAME_LEN - 1;
    for (int i = 0; i < nnames; i++) {
        int j = 0;
        while (j < n && names[i][j] == s[j]) j++;
        if (j == n && names[i][j] == 0) return i;
    }
    if (nnames == MAX_NAMES) return 0;
    for (int j = 0; j < n; j++) names[nnames][j] = s[j];
    names[nnames][n] = 0;
    return nnames++;
}

static bool is_digit(char c) { return c >= '0' && c <= '9'; }
static bool is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }

static int tokenize(const char *src) {
    int i = 0;
    ntoks = 0;
    for (;;) {
        while (src[i] == ' ' || src[i] == '\n' || src[i] == '\t') i++;
        if (ntoks == MAX_TOKENS - 1) return -1;
        struct token *t = &toks[ntoks++];
        t->pos = i;
        t->num = 0;
        t->name = -1;
        char c = src[i];
        if (c == 0) { t->kind = T_END; return ntoks; }
        if (is_digit(c)) {
            int64_t v = 0;
            if (c == '0' && (src[i + 1] == 'x' || src[i + 1] == 'X')) {
                i += 2;
                for (;;) {
                    char d = src[i];
                    int k;
                    if (is_digit(d)) k = d - '0';
                    else if (d >= 'a' && d <= 'f') k = d - 'a' + 10;
                    else if (d >= 'A' && d <= 'F') k = d - 'A' + 10;
                    else break;
                    v = (int64_t)((uint64_t)v * 16u + (uint64_t)k);
                    i++;
                }
            } else {
                while (is_digit(src[i])) { v = (int64_t)((uint64_t)v * 10u + (uint64_t)(src[i] - '0')); i++; }
            }
            t->kind = T_NUM;
            t->num = v;
            continue;
        }
        if (is_alpha(c)) {
            int s = i;
            while (is_alpha(src[i]) || is_digit(src[i])) i++;
            t->kind = T_NAME;
            t->name = intern(src + s, i - s);
            continue;
        }
        i++;
        switch (c) {
        case '+': t->kind = T_PLUS; break;
        case '-': t->kind = T_MINUS; break;
        case '*': t->kind = T_STAR; break;
        case '/': t->kind = T_SLASH; break;
        case '%': t->kind = T_PERCENT; break;
        case '(': t->kind = T_LP; break;
        case ')': t->kind = T_RP; break;
        case '?': t->kind = T_QUEST; break;
        case ':': t->kind = T_COLON; break;
        case ';': t->kind = T_SEMI; break;
        case ',': t->kind = T_COMMA; break;
        case '~': t->kind = T_TILDE; break;
        case '^': t->kind = T_CARET; break;
        case '=':
            if (src[i] == '=') { i++; t->kind = T_EQ; } else t->kind = T_ASSIGN;
            break;
        case '!':
            if (src[i] == '=') { i++; t->kind = T_NE; } else t->kind = T_NOT;
            break;
        case '<':
            if (src[i] == '=') { i++; t->kind = T_LE; }
            else if (src[i] == '<') { i++; t->kind = T_SHL; }
            else t->kind = T_LT;
            break;
        case '>':
            if (src[i] == '=') { i++; t->kind = T_GE; }
            else if (src[i] == '>') { i++; t->kind = T_SHR; }
            else t->kind = T_GT;
            break;
        case '&':
            if (src[i] == '&') { i++; t->kind = T_AND; } else t->kind = T_AMP;
            break;
        case '|':
            if (src[i] == '|') { i++; t->kind = T_OR; } else t->kind = T_PIPE;
            break;
        default: t->kind = T_BAD; break;
        }
    }
}

/* The tree. */
enum nkind { N_NUM, N_VAR, N_BIN, N_UN, N_ASSIGN, N_COND, N_AND, N_OR, N_CALL, N_SEQ };

struct node {
    enum nkind kind;
    enum tok op;
    int64_t num;
    int var;
    int a, b, c;   /* children, -1 when absent */
    int nargs;
    int args[4];
};

#define MAX_NODES 4096
static struct node pool[MAX_NODES];
static int npool;
static int cur;
static int errors;

static int new_node(enum nkind k) {
    if (npool == MAX_NODES) { errors++; return 0; }
    struct node *n = &pool[npool];
    n->kind = k;
    n->op = T_END;
    n->num = 0;
    n->var = -1;
    n->a = n->b = n->c = -1;
    n->nargs = 0;
    return npool++;
}

static enum tok peek(void) { return toks[cur].kind; }
static bool accept(enum tok k) {
    if (toks[cur].kind == k) { cur++; return true; }
    return false;
}
static void expect(enum tok k) {
    if (!accept(k)) errors++;
}

static int parse_expr(void);

static int parse_primary(void) {
    struct token *t = &toks[cur];
    if (t->kind == T_NUM) {
        cur++;
        int n = new_node(N_NUM);
        pool[n].num = t->num;
        return n;
    }
    if (t->kind == T_NAME) {
        cur++;
        if (accept(T_LP)) {
            int n = new_node(N_CALL);
            pool[n].var = t->name;
            if (!accept(T_RP)) {
                do {
                    int e = parse_expr();
                    if (pool[n].nargs < 4) pool[n].args[pool[n].nargs++] = e;
                    else errors++;
                } while (accept(T_COMMA));
                expect(T_RP);
            }
            return n;
        }
        int n = new_node(N_VAR);
        pool[n].var = t->name;
        return n;
    }
    if (accept(T_LP)) {
        int e = parse_expr();
        expect(T_RP);
        return e;
    }
    errors++;
    cur++;
    return new_node(N_NUM);
}

static int parse_unary(void) {
    enum tok k = peek();
    if (k == T_MINUS || k == T_NOT || k == T_TILDE || k == T_PLUS) {
        cur++;
        int n = new_node(N_UN);
        pool[n].op = k;
        pool[n].a = parse_unary();
        return n;
    }
    return parse_primary();
}

/* Binary operators by precedence: a table of levels. */
static const enum tok levels[][4] = {
    { T_PIPE, T_END },
    { T_CARET, T_END },
    { T_AMP, T_END },
    { T_EQ, T_NE, T_END },
    { T_LT, T_LE, T_GT, T_GE },
    { T_SHL, T_SHR, T_END },
    { T_PLUS, T_MINUS, T_END },
    { T_STAR, T_SLASH, T_PERCENT, T_END },
};
#define NLEVELS ((int)(sizeof levels / sizeof levels[0]))

static int parse_binary(int level) {
    if (level == NLEVELS) return parse_unary();
    int lhs = parse_binary(level + 1);
    for (;;) {
        enum tok k = peek();
        bool found = false;
        for (int i = 0; i < 4 && levels[level][i] != T_END; i++)
            if (levels[level][i] == k) found = true;
        if (!found) return lhs;
        cur++;
        int n = new_node(N_BIN);
        pool[n].op = k;
        pool[n].a = lhs;
        pool[n].b = parse_binary(level + 1);
        lhs = n;
    }
}

static int parse_and(void) {
    int lhs = parse_binary(0);
    while (accept(T_AND)) {
        int n = new_node(N_AND);
        pool[n].a = lhs;
        pool[n].b = parse_binary(0);
        lhs = n;
    }
    return lhs;
}

static int parse_or(void) {
    int lhs = parse_and();
    while (accept(T_OR)) {
        int n = new_node(N_OR);
        pool[n].a = lhs;
        pool[n].b = parse_and();
        lhs = n;
    }
    return lhs;
}

static int parse_cond(void) {
    int c = parse_or();
    if (accept(T_QUEST)) {
        int n = new_node(N_COND);
        pool[n].a = c;
        pool[n].b = parse_expr();
        expect(T_COLON);
        pool[n].c = parse_cond();
        return n;
    }
    return c;
}

static int parse_expr(void) {
    int save = cur;
    if (peek() == T_NAME && toks[cur + 1].kind == T_ASSIGN) {
        int v = toks[cur].name;
        cur += 2;
        int n = new_node(N_ASSIGN);
        pool[n].var = v;
        pool[n].a = parse_expr();
        return n;
    }
    cur = save;
    return parse_cond();
}

static int parse_program(void) {
    int first = -1, last = -1;
    while (peek() != T_END) {
        int e = parse_expr();
        if (!accept(T_SEMI) && peek() != T_END) { errors++; cur++; }
        int s = new_node(N_SEQ);
        pool[s].a = e;
        if (last < 0) first = s; else pool[last].b = s;
        last = s;
    }
    return first;
}

/* Arithmetic with C's rules made total: wrapping, and division by zero is 0. */
static int64_t wrap_add(int64_t a, int64_t b) { return (int64_t)((uint64_t)a + (uint64_t)b); }
static int64_t wrap_sub(int64_t a, int64_t b) { return (int64_t)((uint64_t)a - (uint64_t)b); }
static int64_t wrap_mul(int64_t a, int64_t b) { return (int64_t)((uint64_t)a * (uint64_t)b); }
static int64_t safe_div(int64_t a, int64_t b) {
    if (b == 0) return 0;
    if (b == -1) return wrap_sub(0, a);
    return a / b;
}
static int64_t safe_mod(int64_t a, int64_t b) {
    if (b == 0 || b == -1) return 0;
    return a % b;
}

static int64_t binop(enum tok op, int64_t x, int64_t y) {
    switch (op) {
    case T_PLUS: return wrap_add(x, y);
    case T_MINUS: return wrap_sub(x, y);
    case T_STAR: return wrap_mul(x, y);
    case T_SLASH: return safe_div(x, y);
    case T_PERCENT: return safe_mod(x, y);
    case T_EQ: return x == y;
    case T_NE: return x != y;
    case T_LT: return x < y;
    case T_LE: return x <= y;
    case T_GT: return x > y;
    case T_GE: return x >= y;
    case T_SHL: return (int64_t)((uint64_t)x << (y & 63));
    case T_SHR: return x >= 0 ? x >> (y & 63) : ~(~x >> (y & 63));
    case T_AMP: return x & y;
    case T_PIPE: return x | y;
    case T_CARET: return x ^ y;
    default: return 0;
    }
}

static int64_t unop(enum tok op, int64_t x) {
    switch (op) {
    case T_MINUS: return wrap_sub(0, x);
    case T_NOT: return !x;
    case T_TILDE: return ~x;
    default: return x;
    }
}

static int64_t builtin(int name, int n, const int64_t *a) {
    const char *s = names[name];
    if (s[0] == 'm' && s[1] == 'a' && s[2] == 'x' && n == 2) return a[0] > a[1] ? a[0] : a[1];
    if (s[0] == 'm' && s[1] == 'i' && s[2] == 'n' && n == 2) return a[0] < a[1] ? a[0] : a[1];
    if (s[0] == 'a' && s[1] == 'b' && s[2] == 's' && n == 1) return a[0] < 0 ? wrap_sub(0, a[0]) : a[0];
    if (s[0] == 'g' && s[1] == 'c' && s[2] == 'd' && n == 2) {
        uint64_t x = a[0] < 0 ? 0 - (uint64_t)a[0] : (uint64_t)a[0];
        uint64_t y = a[1] < 0 ? 0 - (uint64_t)a[1] : (uint64_t)a[1];
        while (y) { uint64_t t = x % y; x = y; y = t; }
        return (int64_t)x;
    }
    if (s[0] == 'p' && s[1] == 'o' && s[2] == 'w' && n == 2) {
        int64_t r = 1, b = a[0];
        int64_t e = a[1] & 63;
        while (e) { if (e & 1) r = wrap_mul(r, b); b = wrap_mul(b, b); e >>= 1; }
        return r;
    }
    if (s[0] == 'c' && s[1] == 'l' && s[2] == 'z' && n == 1) {
        uint64_t x = (uint64_t)a[0];
        int c = 0;
        if (!x) return 64;
        while (!(x & 0x8000000000000000ull)) { x <<= 1; c++; }
        return c;
    }
    return wrap_add(wrap_mul(n, 1000), n > 0 ? a[0] : 0);
}

static int64_t vars_tree[MAX_NAMES];

static int64_t eval(int n) {
    struct node *p = &pool[n];
    switch (p->kind) {
    case N_NUM: return p->num;
    case N_VAR: return vars_tree[p->var];
    case N_BIN: {
        int64_t x = eval(p->a);
        int64_t y = eval(p->b);
        return binop(p->op, x, y);
    }
    case N_UN: return unop(p->op, eval(p->a));
    case N_ASSIGN: return vars_tree[p->var] = eval(p->a);
    case N_COND: return eval(p->a) ? eval(p->b) : eval(p->c);
    case N_AND: return eval(p->a) && eval(p->b);
    case N_OR: return eval(p->a) || eval(p->b);
    case N_CALL: {
        int64_t args[4] = { 0 };
        for (int i = 0; i < p->nargs; i++) args[i] = eval(p->args[i]);
        return builtin(p->var, p->nargs, args);
    }
    case N_SEQ: {
        int64_t r = 0;
        for (int s = n; s >= 0; s = pool[s].b) r = eval(pool[s].a);
        return r;
    }
    }
    return 0;
}

/* Constant folding: a pass over the tree that replaces operators whose
   operands are numbers. Returns the number of nodes folded. */
static int fold(int n) {
    if (n < 0) return 0;
    struct node *p = &pool[n];
    int k = 0;
    switch (p->kind) {
    case N_BIN:
        k += fold(p->a) + fold(p->b);
        if (pool[p->a].kind == N_NUM && pool[p->b].kind == N_NUM) {
            p->num = binop(p->op, pool[p->a].num, pool[p->b].num);
            p->kind = N_NUM;
            k++;
        }
        break;
    case N_UN:
        k += fold(p->a);
        if (pool[p->a].kind == N_NUM) {
            p->num = unop(p->op, pool[p->a].num);
            p->kind = N_NUM;
            k++;
        }
        break;
    case N_COND:
        k += fold(p->a) + fold(p->b) + fold(p->c);
        if (pool[p->a].kind == N_NUM) {
            int pick = pool[p->a].num ? p->b : p->c;
            *p = pool[pick];
            k++;
        }
        break;
    case N_AND: case N_OR:
        k += fold(p->a) + fold(p->b);
        break;
    case N_ASSIGN: k += fold(p->a); break;
    case N_CALL:
        for (int i = 0; i < p->nargs; i++) k += fold(p->args[i]);
        break;
    case N_SEQ:
        for (int s = n; s >= 0; s = pool[s].b) k += fold(pool[s].a);
        break;
    default: break;
    }
    return k;
}

/* The stack machine. */
enum op {
    OP_PUSH, OP_LOAD, OP_STORE, OP_POP, OP_BIN, OP_UN, OP_JZ, OP_JNZ, OP_JMP,
    OP_CALL, OP_BOOL, OP_HALT
};

struct insn {
    unsigned char op;
    unsigned char arg8;   /* operator token, or argument count */
    short var;
    int target;
    int64_t imm;
};

#define MAX_CODE 8192
static struct insn code[MAX_CODE];
static int ncode;

static int emit(enum op o, int arg8, int var, int64_t imm) {
    if (ncode == MAX_CODE) { errors++; return 0; }
    struct insn *i = &code[ncode];
    i->op = (unsigned char)o;
    i->arg8 = (unsigned char)arg8;
    i->var = (short)var;
    i->target = -1;
    i->imm = imm;
    return ncode++;
}

static void gen(int n) {
    struct node *p = &pool[n];
    switch (p->kind) {
    case N_NUM: emit(OP_PUSH, 0, 0, p->num); break;
    case N_VAR: emit(OP_LOAD, 0, p->var, 0); break;
    case N_BIN: gen(p->a); gen(p->b); emit(OP_BIN, p->op, 0, 0); break;
    case N_UN: gen(p->a); emit(OP_UN, p->op, 0, 0); break;
    case N_ASSIGN: gen(p->a); emit(OP_STORE, 0, p->var, 0); break;
    case N_COND: {
        gen(p->a);
        int jz = emit(OP_JZ, 0, 0, 0);
        gen(p->b);
        int jmp = emit(OP_JMP, 0, 0, 0);
        code[jz].target = ncode;
        gen(p->c);
        code[jmp].target = ncode;
        break;
    }
    case N_AND: case N_OR: {
        gen(p->a);
        emit(OP_BOOL, 0, 0, 0);
        int j = emit(p->kind == N_AND ? OP_JZ : OP_JNZ, 1, 0, 0);
        gen(p->b);
        emit(OP_BOOL, 0, 0, 0);
        code[j].target = ncode;
        break;
    }
    case N_CALL:
        for (int i = 0; i < p->nargs; i++) gen(p->args[i]);
        emit(OP_CALL, p->nargs, p->var, 0);
        break;
    case N_SEQ:
        for (int s = n; s >= 0; s = pool[s].b) {
            gen(pool[s].a);
            if (pool[s].b >= 0) emit(OP_POP, 0, 0, 0);
        }
        break;
    }
}

static int64_t vars_vm[MAX_NAMES];
static int max_depth;

static int64_t run(void) {
    int64_t stack[256];
    int sp = 0;
    int pc = 0;
    for (;;) {
        struct insn *i = &code[pc++];
        if (sp > max_depth) max_depth = sp;
        switch ((enum op)i->op) {
        case OP_PUSH: stack[sp++] = i->imm; break;
        case OP_LOAD: stack[sp++] = vars_vm[i->var]; break;
        case OP_STORE: vars_vm[i->var] = stack[sp - 1]; break;
        case OP_POP: sp--; break;
        case OP_BIN: sp--; stack[sp - 1] = binop((enum tok)i->arg8, stack[sp - 1], stack[sp]); break;
        case OP_UN: stack[sp - 1] = unop((enum tok)i->arg8, stack[sp - 1]); break;
        case OP_BOOL: stack[sp - 1] = stack[sp - 1] != 0; break;
        case OP_JZ:
            /* arg8 = 1: keep the value when jumping (short circuit) */
            if (stack[sp - 1] == 0) { if (!i->arg8) sp--; pc = i->target; }
            else sp--;
            break;
        case OP_JNZ:
            if (stack[sp - 1] != 0) pc = i->target;
            else sp--;
            break;
        case OP_JMP: pc = i->target; break;
        case OP_CALL: {
            int n = i->arg8;
            sp -= n;
            int64_t r = builtin(i->var, n, &stack[sp]);
            stack[sp++] = r;
            break;
        }
        case OP_HALT: return sp > 0 ? stack[sp - 1] : 0;
        }
        if (sp < 0 || sp > 250) return -999;
    }
}

static uint64_t mix(uint64_t h, int64_t v) {
    h ^= (uint64_t)v;
    h *= 0x100000001b3ull;
    return h ^ (h >> 29);
}

static const char *const programs[] = {
    "1 + 2 * 3",
    "(1 + 2) * 3 - 4 / 2 % 3",
    "x = 10; y = x * x - 3; y % 7",
    "a = 5; b = a > 3 ? a * 2 : a - 2; b + (a == 5) + (a != 5)",
    "n = 0; n || (n = 7); n && (n = n + 1); n",
    "max(3, 9) + min(-4, 2) + abs(-17) + gcd(84, 36)",
    "pow(3, 13) - pow(2, 40) + clz(1) + clz(0) + clz(255)",
    "f = 0xFF; g = f << 4 | 3; (g >> 2) ^ ~f & 0x5555",
    "q = -7; r = 2; q / r * r + q % r",
    "big = 9223372036854775807; big + 1",
    "z = 0; 10 / z + 10 % z",
    "t = 1 < 2 < 3; u = 3 > 2 > 1; t * 10 + u",
    "c = 0; c ? 1 : 0 ? 2 : 3",
    "s = 1; s = s * 2 + 1; s = s * 2 + 1; s = s * 2 + 1; s = s * 2 + 1; s",
    "-(-(-(5))) + !0 + !!42 + ~~7",
    "m = 0x7fffffff; m * m * m",
    "p = 2; p = p * p; p = p * p; p = p * p; p = p * p; p = p * p; p = p * p",
    "w = (1 && 0) || (2 && 3) || (z = 99); w * 100 + z",
    "k = 12345678; (k % 1000) * 1000 + k / 100000 % 10",
    "e = 1 ? 2 ? 3 : 4 : 5; e + (0 ? 7 : 1 ? 8 : 9)",
    "h = -1; h >> 3; h << 63",
    "v = gcd(pow(2, 20), pow(6, 8)); clz(v) * 100 + v % 997",
    "a = 3; b = 4; c = a * a + b * b; c == 25 && a < b && !(a > b)",
    "x = 1; x = x + x; x = x + x; y = x; x = y * y + x; max(x, y) - min(x, y)",
    "r = 0; r = r * 31 + 1; r = r * 31 + 2; r = r * 31 + 3; r = r * 31 + 4; r",
    "(((((((((1)))))))))+((((2))))*(((3)))",
    "1 2",
    "foo(1, 2, 3)",
    "max(1)",
    "j = 5; j = j - 10; j * j * j % 13",
};

int main(void) {
    uint64_t h = 0xcbf29ce484222325ull;
    int total_folded = 0, mismatches = 0;
    int nprog = (int)(sizeof programs / sizeof programs[0]);
    for (int p = 0; p < nprog; p++) {
        errors = 0;
        npool = 0;
        cur = 0;
        ncode = 0;
        if (tokenize(programs[p]) < 0) { print_str("too many tokens", 15); continue; }
        int root = parse_program();
        int parse_errors = errors;
        if (root < 0) { h = mix(h, -1); continue; }
        for (int i = 0; i < MAX_NAMES; i++) vars_tree[i] = vars_vm[i] = 0;
        int64_t a = eval(root);
        for (int i = 0; i < MAX_NAMES; i++) vars_tree[i] = 0;
        int f = fold(root);
        total_folded += f;
        int64_t b = eval(root);
        gen(root);
        emit(OP_HALT, 0, 0, 0);
        int64_t c = run();
        if (a != b || b != c) mismatches++;
        h = mix(h, a);
        h = mix(h, parse_errors);
        h = mix(h, ncode);
        if (p % 6 == 0 || parse_errors) {
            print_i64(p);
            print_i64(a);
        }
    }
    /* A generated program: a long chain, to exercise deep recursion. */
    {
        static char buf[4000];
        int n = 0;
        for (int i = 0; i < 300; i++) {
            buf[n++] = '(';
        }
        buf[n++] = '1';
        for (int i = 0; i < 300; i++) {
            buf[n++] = '+';
            buf[n++] = (char)('0' + i % 10);
            buf[n++] = ')';
        }
        buf[n] = 0;
        errors = npool = cur = ncode = 0;
        tokenize(buf);
        int root = parse_program();
        int64_t a = eval(root);
        gen(root);
        emit(OP_HALT, 0, 0, 0);
        int64_t c = run();
        print_i64(a);
        print_i64(c);
        print_i64(npool);
        print_i64(errors);
    }
    print_i64(total_folded);
    print_i64(mismatches);
    print_i64(nnames);
    print_i64(max_depth);
    print_u64(h);
    return 0;
}

/* expect: 0 */
/* expect: 7 */
/* expect: 6 */
/* expect: -1099510033270 */
/* expect: 12 */
/* expect: 3 */
/* expect: 18 */
/* expect: 678003 */
/* expect: 24 */
/* expect: 31810 */
/* expect: 26 */
/* expect: 1 */
/* expect: 1351 */
/* expect: 1351 */
/* expect: 602 */
/* expect: 0 */
/* expect: 33 */
/* expect: 0 */
/* expect: 30 */
/* expect: 3 */
/* expect: 731823140827407299 */
