// parser.c - LL(1) recursive-descent parser.
#include <ctype.h>

#include "ast.h"
#include "builtins.h"
#include "diag.h"

const BuiltinInfo builtin_info[BI_COUNT] = {
#define X(id, name, sig) {name, sig},
    BUILTINS(X)
#undef X
};

int builtin_lookup(const char *name) {
    for (int i = 0; i < BI_COUNT; i++)
        if (strcmp(builtin_info[i].name, name) == 0) return i;
    return -1;
}

typedef struct Parser {
    SourceFile *f;
    Token *t;
    int pos, n;
    Module *mod;
    int errors_at_pos;  // suppress cascades: last position we reported at
    int ntests;         // test blocks so far (their symbol names are per module)
} Parser;

static Token *peek(Parser *P) { return &P->t[P->pos]; }
static Token *peek2(Parser *P) { return &P->t[P->pos + 1 < P->n ? P->pos + 1 : P->n - 1]; }
static Token *peek3(Parser *P) { return &P->t[P->pos + 2 < P->n ? P->pos + 2 : P->n - 1]; }
static TokKind pk(Parser *P) { return P->t[P->pos].kind; }
// `inline` is a keyword only before `fn` (spec §9.5)
static bool at_inline(Parser *P) {
    return pk(P) == TK_IDENT && !strcmp(peek(P)->name, "inline") && peek2(P)->kind == KW_FN;
}

static Token *advance(Parser *P) {
    Token *t = &P->t[P->pos];
    if (P->pos < P->n - 1) P->pos++;
    return t;
}
static bool accept(Parser *P, TokKind k) {
    if (pk(P) == k) {
        advance(P);
        return true;
    }
    return false;
}
static Span prev_span(Parser *P) { return P->t[P->pos > 0 ? P->pos - 1 : 0].span; }

static void syntax_error(Parser *P, const char *what) {
    if (P->errors_at_pos == P->pos) return;
    P->errors_at_pos = P->pos;
    Token *t = peek(P);
    diag_error(t->span, "E0100", "expected %s, found '%s'", what, tok_kind_name(t->kind));
}

static Token *expect(Parser *P, TokKind k) {
    if (pk(P) == k) return advance(P);
    char buf[64];
    snprintf(buf, sizeof buf, "'%s'", tok_kind_name(k));
    syntax_error(P, arena_strndup(buf, strlen(buf)));
    return peek(P);
}

static Sym expect_ident(Parser *P, Span *sp) {
    Token *t = peek(P);
    if (t->kind == TK_IDENT) {
        advance(P);
        if (sp) *sp = t->span;
        return t->name;
    }
    if (sp) *sp = t->span;
    if (t->kind == TK_PRIM || (t->kind >= KW_AND && t->kind <= KW_WHILE)) {
        // An identifier-like word that is reserved: consume it so parsing can continue.
        const char *w = t->kind == TK_PRIM ? prim_names[t->prim] : tok_kind_name(t->kind);
        diag_error(t->span, "E0104", "'%s' is a reserved %s and cannot be used as a name", w,
                   t->kind == TK_PRIM ? "type name" : "keyword");
        P->errors_at_pos = P->pos;
        advance(P);
        return intern_cstr(arena_printf("<reserved %s>", w));
    }
    syntax_error(P, "identifier");
    return intern_cstr("<error>");
}

// Skip to a plausible statement/declaration boundary after an error.
static void sync_stmt(Parser *P) {
    int depth = 0;
    while (pk(P) != TK_EOF) {
        TokKind k = pk(P);
        if (k == TK_LBRACE) depth++;
        if (k == TK_RBRACE) {
            if (depth == 0) return;
            depth--;
        }
        advance(P);
        if (k == TK_SEMI && depth == 0) return;
    }
}

static Span sp_from(Parser *P, Span start) {
    Span e = prev_span(P);
    Span r = start;
    if (e.end > r.end) r.end = e.end;
    return r;
}

static TypeExpr *parse_type(Parser *P);
static Expr *parse_expr(Parser *P);
static Expr *parse_binexpr(Parser *P);
static Stmt *parse_block(Parser *P);
static Stmt *parse_stmt(Parser *P);
static Stmt *parse_if(Parser *P);

// ---------------------------------------------------------------------------
// Types
// ---------------------------------------------------------------------------
static TypeExpr *new_te(TEKind k, Span sp) {
    TypeExpr *t = NEW(TypeExpr);
    t->kind = k;
    t->span = sp;
    return t;
}

static void parse_field_list(Parser *P, FieldVec *out) {
    // Field ( "," Field )* ","?
    while (pk(P) == TK_IDENT && !at_inline(P)) {
        Field f = {0};
        f.name = expect_ident(P, &f.span);
        expect(P, TK_COLON);
        f.te = parse_type(P);
        vec_push(*out, f);
        if (!accept(P, TK_COMMA)) break;
    }
}

static void parse_generic_arg_list(Parser *P, TypeExprVec *out) {
    // after '[' ; GenericArg ( "," GenericArg )* ","? "]"
    if (pk(P) == TK_RBRACKET) {
        syntax_error(P, "generic argument");
    }
    while (pk(P) != TK_RBRACKET && pk(P) != TK_EOF) {
        if (pk(P) == TK_INT) {
            Token *t = advance(P);
            TypeExpr *te = new_te(TE_INTLIT, t->span);
            te->ival = t->ival;
            vec_push(*out, te);
        } else {
            vec_push(*out, parse_type(P));
        }
        if (!accept(P, TK_COMMA)) break;
    }
    expect(P, TK_RBRACKET);
}

static TypeExpr *parse_type(Parser *P) {
    Token *t = peek(P);
    Span start = t->span;
    switch (t->kind) {
    case TK_BANG: {
        advance(P);
        TypeExpr *te = new_te(TE_ERRUNION, start);
        te->elem = parse_type(P);
        te->span = sp_from(P, start);
        return te;
    }
    case TK_STAR: {
        advance(P);
        TypeExpr *te = new_te(TE_PTR, start);
        te->mut = accept(P, KW_MUT);
        te->elem = parse_type(P);
        te->span = sp_from(P, start);
        return te;
    }
    case TK_LBRACKET: {
        advance(P);
        if (accept(P, TK_RBRACKET)) {
            TypeExpr *te = new_te(TE_SLICE, start);
            te->mut = accept(P, KW_MUT);
            te->elem = parse_type(P);
            te->span = sp_from(P, start);
            return te;
        }
        // the length: an integer literal, a name (a const or a const
        // parameter, perhaps qualified), or any other constant expression
        TypeExpr *te = new_te(TE_ARRAY, start);
        bool qualified = pk(P) == TK_IDENT && peek2(P)->kind == TK_DOT && peek3(P)->kind == TK_IDENT;
        if (pk(P) == TK_INT && peek2(P)->kind == TK_RBRACKET) {
            Token *n = advance(P);
            te->len = new_te(TE_INTLIT, n->span);
            te->len->ival = n->ival;
        } else if (pk(P) == TK_IDENT && (peek2(P)->kind == TK_RBRACKET || qualified)) {
            int save = P->pos;
            Token *n = advance(P);
            te->len = new_te(TE_NAMED, n->span);
            te->len->name = n->name;
            te->len->name_span = n->span;
            if (qualified) {
                advance(P);
                Token *m = advance(P);
                te->len->qual = te->len->name;
                te->len->name = m->name;
                te->len->name_span = m->span;
                te->len->span = span_join(n->span, m->span);
            }
            if (pk(P) != TK_RBRACKET) {  // an expression after all
                P->pos = save;
                te->len = NULL;
            }
        }
        if (!te->len) {
            if (pk(P) == TK_RBRACKET) {
                syntax_error(P, "array length");
            } else {
                Expr *e = parse_expr(P);
                te->len = new_te(TE_CONSTEXPR, e->span);
                te->len->expr = e;
            }
        }
        expect(P, TK_RBRACKET);
        te->elem = parse_type(P);
        te->span = sp_from(P, start);
        return te;
    }
    case KW_FN: {
        advance(P);
        TypeExpr *te = new_te(TE_FN, start);
        expect(P, TK_LPAREN);
        while (pk(P) != TK_RPAREN && pk(P) != TK_EOF) {
            vec_push(te->args, parse_type(P));
            if (!accept(P, TK_COMMA)) break;
        }
        expect(P, TK_RPAREN);
        te->elem = parse_type(P);
        te->span = sp_from(P, start);
        return te;
    }
    case KW_STRUCT: {
        advance(P);
        TypeExpr *te = new_te(TE_ANON_STRUCT, start);
        expect(P, TK_LBRACE);
        parse_field_list(P, &te->fields);
        expect(P, TK_RBRACE);
        te->span = sp_from(P, start);
        return te;
    }
    case KW_ERROR: advance(P); return new_te(TE_ERROR, start);
    case KW_SELF: advance(P); return new_te(TE_SELF, start);
    case TK_PRIM: {
        advance(P);
        TypeExpr *te = new_te(TE_PRIM, start);
        te->prim = t->prim;
        return te;
    }
    case TK_IDENT: {
        advance(P);
        TypeExpr *te = new_te(TE_NAMED, start);
        te->name = t->name;
        te->name_span = t->span;
        // NamedType ::= IDENT ( "." ( IDENT GenericArgs? | "[" GenericArgList "]" ) )?
        if (pk(P) == TK_DOT) {
            TokKind k2 = peek2(P)->kind;
            if (k2 == TK_IDENT) {
                advance(P);
                Token *m = advance(P);
                te->qual = te->name;
                te->name = m->name;
                te->name_span = m->span;
                if (pk(P) == TK_DOT && peek2(P)->kind == TK_LBRACKET) {
                    advance(P);
                    advance(P);
                    te->has_args = true;
                    parse_generic_arg_list(P, &te->args);
                }
            } else if (k2 == TK_LBRACKET) {
                advance(P);
                advance(P);
                te->has_args = true;
                parse_generic_arg_list(P, &te->args);
            }
        }
        te->span = sp_from(P, start);
        return te;
    }
    default:
        syntax_error(P, "type");
        return new_te(TE_ERROR, start);
    }
}

// ---------------------------------------------------------------------------
// Expressions
// ---------------------------------------------------------------------------
static Expr *new_expr(ExprKind k, Span sp) {
    Expr *e = NEW(Expr);
    e->kind = k;
    e->span = sp;
    return e;
}

static void parse_field_inits(Parser *P, Expr *e) {
    // after '{'
    while (pk(P) == TK_IDENT) {
        FieldInit fi = {0};
        fi.name = expect_ident(P, &fi.span);
        expect(P, TK_COLON);
        fi.value = parse_expr(P);
        vec_push(e->inits, fi);
        if (!accept(P, TK_COMMA)) break;
    }
    expect(P, TK_RBRACE);
}

static Expr *parse_builtin(Parser *P) {
    Token *t = advance(P);
    Expr *e = new_expr(E_BUILTIN, t->span);
    e->name = t->name;
    int id = builtin_lookup(t->name);
    if (id < 0) diag_error(t->span, "E0120", "unknown builtin '@%s'", t->name);
    e->index = id;
    const char *sig = id >= 0 ? builtin_info[id].sig : "*";
    expect(P, TK_LPAREN);
    while (pk(P) != TK_RPAREN && pk(P) != TK_EOF) {
        char kind = sig[0] == '*' ? 'e' : sig[0] ? sig[0] : 'e';
        if (sig[0] && sig[0] != '*') sig++;
        BuiltinArg a = {0};
        if (kind == 't') a.t = parse_type(P);
        else a.e = parse_expr(P);
        vec_push(e->bargs, a);
        if (!accept(P, TK_COMMA)) break;
    }
    expect(P, TK_RPAREN);
    e->span = sp_from(P, t->span);
    return e;
}

static Expr *parse_primary(Parser *P) {
    Token *t = peek(P);
    Span start = t->span;
    switch (t->kind) {
    case TK_IDENT: {
        advance(P);
        Expr *e = new_expr(E_IDENT, start);
        e->name = t->name;
        return e;
    }
    case KW_SELF: advance(P); return new_expr(E_SELF, start);
    case TK_INT:
    case TK_CHAR: {
        advance(P);
        Expr *e = new_expr(E_INT, start);
        e->ival = t->ival;
        return e;
    }
    case TK_FLOAT: {
        advance(P);
        Expr *e = new_expr(E_FLOAT, start);
        e->fval = t->fval;
        e->float_text = t->float_text;
        return e;
    }
    case TK_STRING: {
        advance(P);
        Expr *e = new_expr(E_STRING, start);
        e->str = t->str.bytes;
        e->slen = t->str.len;
        return e;
    }
    case KW_TRUE:
    case KW_FALSE: {
        advance(P);
        Expr *e = new_expr(E_BOOL, start);
        e->bval = t->kind == KW_TRUE;
        return e;
    }
    case TK_LPAREN: {
        advance(P);
        Expr *e = parse_expr(P);
        expect(P, TK_RPAREN);
        e->parens = true;
        e->span = sp_from(P, start);
        return e;
    }
    case TK_DOT: {
        advance(P);
        expect(P, TK_LBRACE);
        Expr *e = new_expr(E_STRUCT_LIT, start);
        parse_field_inits(P, e);
        e->span = sp_from(P, start);
        return e;
    }
    case TK_LBRACKET: {
        advance(P);
        Expr *first = parse_expr(P);
        Expr *e;
        if (accept(P, TK_SEMI)) {
            e = new_expr(E_ARRAY_REPEAT, start);
            e->a = first;
            e->b = parse_expr(P);
        } else {
            e = new_expr(E_ARRAY_LIT, start);
            vec_push(e->args, first);
            while (accept(P, TK_COMMA)) {
                if (pk(P) == TK_RBRACKET) break;
                vec_push(e->args, parse_expr(P));
            }
        }
        expect(P, TK_RBRACKET);
        e->span = sp_from(P, start);
        return e;
    }
    case KW_ERROR: {
        advance(P);
        expect(P, TK_DOT);
        Expr *e = new_expr(E_ERROR_VAL, start);
        e->name = expect_ident(P, &e->name_span);
        e->span = sp_from(P, start);
        return e;
    }
    case TK_BUILTIN: return parse_builtin(P);
    default:
        syntax_error(P, "expression");
        if (pk(P) != TK_EOF && pk(P) != TK_RBRACE && pk(P) != TK_SEMI) advance(P);
        return new_expr(E_INT, start);
    }
}

static Expr *parse_postfix(Parser *P) {
    Expr *e = parse_primary(P);
    Span start = e->span;
    for (;;) {
        TokKind k = pk(P);
        if (k == TK_DOT) {
            advance(P);
            TokKind k2 = pk(P);
            if (k2 == TK_IDENT) {
                Token *n = advance(P);
                Expr *f = new_expr(E_FIELD, start);
                f->a = e;
                f->name = n->name;
                f->name_span = n->span;
                e = f;
            } else if (k2 == TK_STAR) {
                advance(P);
                Expr *d = new_expr(E_DEREF, start);
                d->a = e;
                e = d;
            } else if (k2 == TK_LBRACE) {
                advance(P);
                Expr *s = new_expr(E_STRUCT_LIT, start);
                s->a = e;
                parse_field_inits(P, s);
                e = s;
            } else if (k2 == TK_LBRACKET) {
                advance(P);
                Expr *ap = new_expr(E_APPLY, start);
                ap->a = e;
                parse_generic_arg_list(P, &ap->targs);
                e = ap;
            } else {
                syntax_error(P, "field name, '*', '{' or '[' after '.'");
                return e;
            }
        } else if (k == TK_LPAREN) {
            advance(P);
            Expr *c = new_expr(E_CALL, start);
            c->a = e;
            while (pk(P) != TK_RPAREN && pk(P) != TK_EOF) {
                vec_push(c->args, parse_expr(P));
                if (!accept(P, TK_COMMA)) break;
            }
            expect(P, TK_RPAREN);
            e = c;
        } else if (k == TK_LBRACKET) {
            advance(P);
            Expr *lo = NULL, *hi = NULL;
            bool is_slice = false;
            if (accept(P, TK_DOTDOT)) {
                is_slice = true;
                if (pk(P) != TK_RBRACKET) hi = parse_expr(P);
            } else {
                lo = parse_expr(P);
                if (accept(P, TK_DOTDOT)) {
                    is_slice = true;
                    if (pk(P) != TK_RBRACKET) hi = parse_expr(P);
                }
            }
            expect(P, TK_RBRACKET);
            Expr *x = new_expr(is_slice ? E_SLICE : E_INDEX, start);
            x->a = e;
            x->b = lo;
            x->c = hi;
            e = x;
        } else if (k == TK_BANG) {
            advance(P);
            Expr *x = new_expr(E_TRY, start);
            x->a = e;
            e = x;
        } else {
            break;
        }
        e->span = sp_from(P, start);
    }
    return e;
}

static Expr *parse_unary(Parser *P) {
    Token *t = peek(P);
    Span start = t->span;
    TokKind k = t->kind;
    if (k == TK_MINUS || k == TK_MINUS_PCT || k == TK_TILDE || k == KW_NOT || k == TK_AMP) {
        advance(P);
        Expr *e = new_expr(E_UNARY, start);
        e->op = k;
        if (k == TK_AMP && accept(P, KW_MUT)) e->bval = true;  // &mut
        e->a = parse_unary(P);
        e->span = sp_from(P, start);
        return e;
    }
    return parse_postfix(P);
}

// --- operator partial order (spec §7.4) ---
enum { F_NONE, F_M, F_A, F_S, F_BAND, F_BOR, F_BXOR, F_C, F_AND, F_OR };

static int op_family(TokKind k) {
    switch (k) {
    case TK_STAR: case TK_SLASH: case TK_PERCENT: case TK_STAR_PCT: return F_M;
    case TK_PLUS: case TK_MINUS: case TK_PLUS_PCT: case TK_MINUS_PCT: return F_A;
    case TK_SHL: case TK_SHR: return F_S;
    case TK_AMP: return F_BAND;
    case TK_PIPE: return F_BOR;
    case TK_CARET: return F_BXOR;
    case TK_EQEQ: case TK_NE: case TK_LT: case TK_LE: case TK_GT: case TK_GE: return F_C;
    case KW_AND: return F_AND;
    case KW_OR: return F_OR;
    default: return F_NONE;
    }
}

static int fam_level(int f) {
    switch (f) {
    case F_M: return 5;
    case F_A: case F_S: case F_BAND: case F_BOR: case F_BXOR: return 4;
    case F_C: return 3;
    case F_AND: return 2;
    case F_OR: return 1;
    default: return 0;
    }
}

// x binds strictly tighter than y in the partial order.
static bool fam_tighter(int x, int y) {
    bool x_arith = x == F_M || x == F_A || x == F_S || x == F_BAND || x == F_BOR || x == F_BXOR;
    if (x == F_M && y == F_A) return true;
    if (x_arith && y == F_C) return true;
    if ((x_arith || x == F_C) && (y == F_AND || y == F_OR)) return true;
    return false;
}

static bool fam_chainable(int f) { return f != F_S && f != F_C && f != F_NONE; }

static void check_mix(Expr *parent, Expr *child, bool is_left) {
    if (child->kind != E_BINARY || child->parens) return;
    int pf = op_family(parent->op), cf = op_family(child->op);
    if (fam_tighter(cf, pf)) return;
    if (cf == pf && fam_chainable(pf) && is_left) return;
    Diag *d = diag_error(child->span, "E0150", "operators '%s' and '%s' cannot be mixed without parentheses",
                         tok_kind_name((TokKind)child->op), tok_kind_name((TokKind)parent->op));
    DiagFix *fx = diag_fix(d, "add parentheses", "machine");
    diag_fix_edit(fx, (Span){child->span.file, child->span.start, child->span.start}, "(");
    diag_fix_edit(fx, (Span){child->span.file, child->span.end, child->span.end}, ")");
}

static Expr *parse_binexpr(Parser *P) {
    Vec(Expr *) operands = {0};
    Vec(TokKind) ops = {0};
    vec_push(operands, parse_unary(P));
    while (op_family(pk(P)) != F_NONE) {
        vec_push(ops, advance(P)->kind);
        vec_push(operands, parse_unary(P));
    }
    if (ops.len == 0) {
        Expr *e = operands.data[0];
        free(operands.data);
        return e;
    }
    // shunting-yard with levels; left-associative within a level
    Vec(Expr *) out = {0};
    Vec(TokKind) st = {0};
    vec_push(out, operands.data[0]);
#define REDUCE()                                                  \
    do {                                                          \
        TokKind op_ = vec_pop(st);                                \
        Expr *r_ = vec_pop(out), *l_ = vec_pop(out);              \
        Expr *b_ = new_expr(E_BINARY, span_join(l_->span, r_->span)); \
        b_->op = op_;                                             \
        b_->a = l_;                                               \
        b_->b = r_;                                               \
        check_mix(b_, l_, true);                                  \
        check_mix(b_, r_, false);                                 \
        vec_push(out, b_);                                        \
    } while (0)
    for (int i = 0; i < ops.len; i++) {
        int lv = fam_level(op_family(ops.data[i]));
        while (st.len && fam_level(op_family(vec_last(st))) >= lv) REDUCE();
        vec_push(st, ops.data[i]);
        vec_push(out, operands.data[i + 1]);
    }
    while (st.len) REDUCE();
#undef REDUCE
    Expr *res = out.data[0];
    free(out.data);
    free(st.data);
    free(operands.data);
    free(ops.data);
    return res;
}

static Expr *parse_expr(Parser *P) {
    Expr *e = parse_binexpr(P);
    if (pk(P) != KW_CATCH) return e;
    Span start = e->span;
    if (e->kind == E_BINARY && !e->parens) {
        Diag *d = diag_error(e->span, "E0150", "'catch' applies to the whole expression '... %s ...'; add parentheses",
                             tok_kind_name((TokKind)e->op));
        DiagFix *fx = diag_fix(d, "catch only the right operand", "suggestion");
        diag_fix_edit(fx, (Span){e->b->span.file, e->b->span.start, e->b->span.start}, "(");
        (void)fx;
    }
    advance(P);
    Expr *c = new_expr(E_CATCH, start);
    c->a = e;
    if (accept(P, TK_PIPE)) {
        c->capture = expect_ident(P, &c->capture_span);
        expect(P, TK_PIPE);
    }
    switch (pk(P)) {
    case KW_RETURN:
        advance(P);
        c->hkind = H_RETURN;
        if (pk(P) != TK_SEMI && pk(P) != TK_RPAREN && pk(P) != TK_COMMA && pk(P) != TK_RBRACE) c->hexpr = parse_expr(P);
        break;
    case KW_BREAK:
    case KW_CONTINUE:
        c->hkind = advance(P)->kind == KW_BREAK ? H_BREAK : H_CONTINUE;
        if (pk(P) == TK_IDENT) {
            Token *id = advance(P);  // the label of the loop to leave (name, as for an identifier)
            c->name = id->name;
            c->name_span = id->span;
        }
        break;
    case TK_LBRACE: c->hkind = H_BLOCK; c->hblock = parse_block(P); break;
    default: c->hkind = H_VALUE; c->hexpr = parse_binexpr(P); break;
    }
    c->span = sp_from(P, start);
    return c;
}

// ---------------------------------------------------------------------------
// Statements
// ---------------------------------------------------------------------------
static Stmt *new_stmt(StmtKind k, Span sp) {
    Stmt *s = NEW(Stmt);
    s->kind = k;
    s->span = sp;
    return s;
}

static Stmt *parse_block(Parser *P) {
    Span start = peek(P)->span;
    Stmt *b = new_stmt(S_BLOCK, start);
    expect(P, TK_LBRACE);
    while (pk(P) != TK_RBRACE && pk(P) != TK_EOF) {
        int before = P->pos;
        Stmt *s = parse_stmt(P);
        if (s) vec_push(b->stmts, s);
        if (P->pos == before) advance(P);  // guarantee progress
    }
    expect(P, TK_RBRACE);
    b->span = sp_from(P, start);
    return b;
}

static bool is_assign_op(TokKind k) {
    switch (k) {
    case TK_EQ: case TK_PLUS_EQ: case TK_MINUS_EQ: case TK_STAR_EQ: case TK_SLASH_EQ: case TK_PERCENT_EQ:
    case TK_PLUS_PCT_EQ: case TK_MINUS_PCT_EQ: case TK_STAR_PCT_EQ: case TK_AMP_EQ: case TK_PIPE_EQ:
    case TK_CARET_EQ: case TK_SHL_EQ: case TK_SHR_EQ: return true;
    default: return false;
    }
}

// ExprStmt ::= Expr ( AssignOp Expr )? ";"
static Stmt *parse_expr_stmt(Parser *P) {
    Span start = peek(P)->span;
    Expr *e = parse_expr(P);
    Stmt *s;
    if (is_assign_op(pk(P))) {
        s = new_stmt(S_ASSIGN, start);
        s->op = advance(P)->kind;
        s->e = e;
        s->e2 = parse_expr(P);
    } else {
        s = new_stmt(S_EXPR, start);
        s->e = e;
    }
    if (!accept(P, TK_SEMI)) {
        syntax_error(P, "';'");
        sync_stmt(P);
    }
    s->span = sp_from(P, start);
    return s;
}

// Pattern ::= "." IDENT ( "(" IDENT ")" )? | "error" "." IDENT | "-"? INT_LIT | CHAR_LIT
//           | "true" | "false" | "else"
static bool parse_pattern(Parser *P, MatchArm *arm, bool allow_else) {
    arm->span = peek(P)->span;
    if (accept(P, TK_DOT)) {
        arm->kind = P_VARIANT;
        arm->name = expect_ident(P, NULL);
        if (accept(P, TK_LPAREN)) {
            arm->bind = expect_ident(P, &arm->bind_span);
            expect(P, TK_RPAREN);
        }
    } else if (accept(P, KW_ERROR)) {
        arm->kind = P_ERROR;
        expect(P, TK_DOT);
        arm->name = expect_ident(P, NULL);
    } else if (pk(P) == TK_MINUS || pk(P) == TK_INT || pk(P) == TK_CHAR) {
        bool neg = accept(P, TK_MINUS);
        arm->kind = P_INT;
        Token *n = peek(P);
        if (n->kind == TK_INT || n->kind == TK_CHAR) {
            advance(P);
            arm->ival = neg ? -(i128)n->ival : (i128)n->ival;
        } else {
            syntax_error(P, "integer literal");
        }
    } else if (pk(P) == KW_TRUE || pk(P) == KW_FALSE) {
        arm->kind = P_BOOL;
        arm->bval = advance(P)->kind == KW_TRUE;
    } else if (allow_else && accept(P, KW_ELSE)) {
        arm->kind = P_ELSE;
    } else {
        syntax_error(P, "match pattern");
        return false;
    }
    arm->span = sp_from(P, arm->span);
    return true;
}

// Pattern ( "," Pattern )*, one arm per pattern (all but the first `alt`);
// `else` only alone. False after a syntax error.
static bool parse_arm_patterns(Parser *P, Stmt *s, bool allow_else) {
    int first = s->arms.len;
    MatchArm arm = {0};
    if (!parse_pattern(P, &arm, allow_else)) return false;
    vec_push(s->arms, arm);
    if (arm.kind == P_ELSE) return true;
    while (accept(P, TK_COMMA)) {
        MatchArm alt = {0};
        alt.alt = true;
        if (!parse_pattern(P, &alt, false)) {
            s->arms.len = first;  // (the arm is dropped)
            return false;
        }
        vec_push(s->arms, alt);
    }
    return true;
}

// The body of the arm just parsed, and of its other patterns.
static void set_arm_body(Stmt *s, Stmt *body) {
    for (int i = s->arms.len - 1; i >= 0; i--) {
        s->arms.data[i].body = body;
        if (!s->arms.data[i].alt) break;
    }
}

// if let Pattern ( "," Pattern )* = Expr Block ( else ( IfStmt | Block ) )?
// is `match Expr { Pattern => Block, else => ... }`.
static Stmt *parse_if_let(Parser *P, Span start) {
    expect(P, KW_LET);
    Stmt *s = new_stmt(S_MATCH, start);
    if (!parse_arm_patterns(P, s, false)) {
        sync_stmt(P);
        return NULL;
    }
    expect(P, TK_EQ);
    s->e = parse_expr(P);
    set_arm_body(s, parse_block(P));
    MatchArm els = {0};
    els.kind = P_ELSE;
    els.span = peek(P)->span;
    if (accept(P, KW_ELSE)) {
        if (pk(P) == KW_IF) {
            Span sp = peek(P)->span;
            Stmt *inner = parse_if(P);
            els.body = new_stmt(S_BLOCK, inner ? inner->span : sp);
            if (inner) vec_push(els.body->stmts, inner);
        } else {
            els.body = parse_block(P);
        }
    } else {
        els.body = new_stmt(S_BLOCK, els.span);
    }
    vec_push(s->arms, els);
    s->span = sp_from(P, start);
    return s;
}

static Stmt *parse_if(Parser *P) {
    Span start = advance(P)->span;
    if (pk(P) == KW_LET) return parse_if_let(P, start);
    Stmt *s = new_stmt(S_IF, start);
    s->e = parse_expr(P);
    s->body = parse_block(P);
    if (accept(P, KW_ELSE)) {
        if (pk(P) == KW_IF) s->els = parse_if(P);
        else s->els = parse_block(P);
    }
    s->span = sp_from(P, start);
    return s;
}

static Stmt *parse_stmt(Parser *P) {
    Token *t = peek(P);
    Span start = t->span;
    if (t->kind == TK_IDENT && peek2(P)->kind == TK_COLON) {
        // a loop label: `name: while|loop|for`
        int k3 = peek3(P)->kind;
        if (k3 == KW_WHILE || k3 == KW_LOOP || k3 == KW_FOR) {
            Token *id = advance(P);
            advance(P);
            Stmt *s = parse_stmt(P);
            s->label = id->name;
            s->label_span = id->span;
            s->span = sp_from(P, start);
            return s;
        }
    }
    switch (t->kind) {
    case KW_LET: {
        advance(P);
        Stmt *s = new_stmt(S_LET, start);
        s->mut = accept(P, KW_MUT);
        s->name = expect_ident(P, &s->name_span);
        if (accept(P, TK_COLON)) s->te = parse_type(P);
        expect(P, TK_EQ);
        // `undefined` (contextual) as the whole initializer: the variable's bytes left
        // unspecified until written
        s->undef = pk(P) == TK_IDENT && !strcmp(peek(P)->name, "undefined") && peek2(P)->kind == TK_SEMI;
        s->e = parse_expr(P);
        if (!accept(P, TK_SEMI)) {
            syntax_error(P, "';'");
            sync_stmt(P);
        }
        s->span = sp_from(P, start);
        return s;
    }
    case KW_IF: return parse_if(P);
    case KW_WHILE: {
        advance(P);
        Stmt *s = new_stmt(S_WHILE, start);
        s->e = parse_expr(P);
        s->body = parse_block(P);
        s->span = sp_from(P, start);
        return s;
    }
    case KW_LOOP: {
        advance(P);
        Stmt *s = new_stmt(S_LOOP, start);
        s->body = parse_block(P);
        s->span = sp_from(P, start);
        return s;
    }
    case KW_FOR: {
        advance(P);
        Stmt *s = new_stmt(S_FOR, start);
        s->name = expect_ident(P, &s->name_span);
        expect(P, KW_IN);
        s->e = parse_expr(P);
        if (accept(P, TK_DOTDOT)) s->e2 = parse_expr(P);
        s->body = parse_block(P);
        s->span = sp_from(P, start);
        return s;
    }
    case KW_MATCH: {
        advance(P);
        Stmt *s = new_stmt(S_MATCH, start);
        s->e = parse_expr(P);
        expect(P, TK_LBRACE);
        while (pk(P) != TK_RBRACE && pk(P) != TK_EOF) {
            if (!parse_arm_patterns(P, s, true)) {
                sync_stmt(P);
                break;
            }
            expect(P, TK_FATARROW);
            set_arm_body(s, parse_block(P));
            accept(P, TK_COMMA);
        }
        expect(P, TK_RBRACE);
        s->span = sp_from(P, start);
        return s;
    }
    case KW_RETURN: {
        advance(P);
        Stmt *s = new_stmt(S_RETURN, start);
        if (pk(P) != TK_SEMI) s->e = parse_expr(P);
        expect(P, TK_SEMI);
        s->span = sp_from(P, start);
        return s;
    }
    case KW_BREAK:
    case KW_CONTINUE: {
        advance(P);
        Stmt *s = new_stmt(t->kind == KW_BREAK ? S_BREAK : S_CONTINUE, start);
        if (pk(P) == TK_IDENT) {
            Token *id = advance(P);  // the label of the loop to leave
            s->label = id->name;
            s->label_span = id->span;
        }
        expect(P, TK_SEMI);
        s->span = sp_from(P, start);
        return s;
    }
    case KW_DEFER:
    case KW_ERRDEFER: {
        Stmt *s = new_stmt(S_DEFER, start);
        s->on_error = advance(P)->kind == KW_ERRDEFER;
        s->body = pk(P) == TK_LBRACE ? parse_block(P) : parse_expr_stmt(P);
        s->span = sp_from(P, start);
        return s;
    }
    case TK_DISCARD: {
        advance(P);
        Stmt *s = new_stmt(S_DISCARD, start);
        expect(P, TK_EQ);
        s->e = parse_expr(P);
        expect(P, TK_SEMI);
        s->span = sp_from(P, start);
        return s;
    }
    case TK_LBRACE: return parse_block(P);
    default: return parse_expr_stmt(P);
    }
}

// ---------------------------------------------------------------------------
// Declarations
// ---------------------------------------------------------------------------
static Decl *new_decl(Parser *P, DeclKind k, Span sp) {
    Decl *d = NEW(Decl);
    d->kind = k;
    d->span = sp;
    d->module = P->mod;
    return d;
}

static void parse_generic_params(Parser *P, Decl *d) {
    if (!accept(P, TK_LBRACKET)) return;
    while (pk(P) != TK_RBRACKET && pk(P) != TK_EOF) {
        GenericParam g = {0};
        if (accept(P, KW_CONST)) {
            g.is_const = true;
            g.name = expect_ident(P, &g.span);
            expect(P, TK_COLON);
            g.const_te = parse_type(P);
        } else {
            g.name = expect_ident(P, &g.span);
            if (accept(P, TK_COLON)) g.constraint = expect_ident(P, &g.constraint_span);
        }
        vec_push(d->gparams, g);
        if (!accept(P, TK_COMMA)) break;
    }
    expect(P, TK_RBRACKET);
}

static void parse_params(Parser *P, Decl *d) {
    expect(P, TK_LPAREN);
    while (pk(P) != TK_RPAREN && pk(P) != TK_EOF) {
        if (pk(P) == TK_ELLIPSIS) {  // C varargs; only valid in extern "C" (checked by sema)
            d->variadic = true;
            d->abi_span = advance(P)->span;
            break;
        }
        Param p = {0};
        p.name = expect_ident(P, &p.span);
        expect(P, TK_COLON);
        p.te = parse_type(P);
        vec_push(d->params, p);
        if (!accept(P, TK_COMMA)) break;
    }
    expect(P, TK_RPAREN);
}

// FnAttr ::= ( "@features" | "@multiversion" ) "(" STRING_LIT ( "," STRING_LIT )* ","? ")"
static void parse_fn_attr(Parser *P, Decl *d) {
    Token *t = advance(P);
    int kind = ATTR_NONE;
    if (!strcmp(t->name, "features")) kind = ATTR_FEATURES;
    else if (!strcmp(t->name, "multiversion")) kind = ATTR_MULTIVERSION;
    else diag_error(t->span, "E0829", "unknown function attribute '@%s' (expected @features or @multiversion)", t->name);
    if (kind != ATTR_NONE && d->attr != ATTR_NONE) {
        diag_error(t->span, "E0829", "a function has at most one attribute (@features or @multiversion)");
        kind = ATTR_NONE;
    }
    Span ts = t->span;
    if (kind != ATTR_NONE) {
        d->attr = kind;
        d->attr_span = ts;
    }
    expect(P, TK_LPAREN);
    while (pk(P) == TK_STRING) {
        Token *s = advance(P);
        if (kind != ATTR_NONE) {
            vec_push(d->attr_args, intern(s->str.bytes, s->str.len));
            vec_push(d->attr_arg_spans, s->span);
        }
        if (!accept(P, TK_COMMA)) break;
    }
    if (pk(P) != TK_RPAREN) syntax_error(P, "a string (the extensions)");
    expect(P, TK_RPAREN);
    if (kind != ATTR_NONE) d->attr_span = sp_from(P, ts);
}

static Decl *parse_fn(Parser *P, Span start, bool is_pub, bool is_export) {
    expect(P, KW_FN);
    Decl *d = new_decl(P, D_FN, start);
    d->is_pub = is_pub;
    d->is_export = is_export;
    d->name = expect_ident(P, &d->name_span);
    parse_generic_params(P, d);
    parse_params(P, d);
    if (pk(P) == TK_LBRACE) {
        Diag *dg = diag_error(peek(P)->span, "E0301", "missing return type (write 'void' if the function returns nothing)");
        DiagFix *fx = diag_fix(dg, "declare a void return type", "suggestion");
        diag_fix_edit(fx, (Span){peek(P)->span.file, peek(P)->span.start, peek(P)->span.start}, "void ");
    } else {
        d->ret_te = parse_type(P);
    }
    while (pk(P) == TK_BUILTIN) parse_fn_attr(P, d);
    d->body = parse_block(P);
    d->span = sp_from(P, start);
    return d;
}

static Decl *parse_fn_inline(Parser *P, Span start, bool is_pub, bool is_export) {
    Span is = peek(P)->span;
    bool inl = at_inline(P);
    if (inl) advance(P);
    Decl *d = parse_fn(P, start, is_pub, is_export);
    d->is_inline = inl;
    d->inline_span = is;
    return d;
}

static void parse_methods(Parser *P, Decl *owner) {
    while (pk(P) == KW_PUB || pk(P) == KW_FN || at_inline(P)) {
        Span ms = peek(P)->span;
        bool mp = accept(P, KW_PUB);
        Decl *m = parse_fn_inline(P, ms, mp, false);
        m->owner = owner;
        vec_push(owner->methods, m);
    }
}

static void parse_top(Parser *P) {
    Token *t = peek(P);
    Span start = t->span;
    if (t->kind == KW_IMPORT) {
        advance(P);
        if (accept(P, KW_HOST)) {
            Sym modname = intern_cstr("env");
            if (pk(P) == TK_STRING) {
                Token *s = advance(P);
                modname = intern(s->str.bytes, s->str.len);
            }
            expect(P, TK_LBRACE);
            while (pk(P) == KW_FN) {
                Span fs = advance(P)->span;
                Decl *d = new_decl(P, D_HOST_FN, fs);
                d->host_module = modname;
                d->name = expect_ident(P, &d->name_span);
                parse_params(P, d);
                d->ret_te = parse_type(P);
                expect(P, TK_SEMI);
                d->span = sp_from(P, fs);
                vec_push(P->mod->decls, d);
            }
            expect(P, TK_RBRACE);
            return;
        }
        Decl *d = new_decl(P, D_IMPORT, start);
        Token *s = expect(P, TK_STRING);
        if (s->kind == TK_STRING) {
            d->import_path = intern(s->str.bytes, s->str.len);
            d->import_path_span = s->span;
        }
        expect(P, KW_AS);
        d->name = expect_ident(P, &d->name_span);
        expect(P, TK_SEMI);
        d->span = sp_from(P, start);
        vec_push(P->mod->decls, d);
        return;
    }
    if (t->kind == KW_ERROR) {
        advance(P);
        Decl *d = new_decl(P, D_ERROR, start);
        expect(P, TK_LBRACE);
        while (pk(P) == TK_IDENT) {
            Span sp;
            Sym n = expect_ident(P, &sp);
            vec_push(d->error_names, n);
            vec_push(d->error_spans, sp);
            if (!accept(P, TK_COMMA)) break;
        }
        expect(P, TK_RBRACE);
        d->span = sp_from(P, start);
        vec_push(P->mod->decls, d);
        return;
    }
    if (t->kind == KW_TEST) {
        // TestDecl ::= "test" STRING_LIT ( "expect" "trap" ( "(" IDENT ")" )? )? Block
        advance(P);
        Decl *d = new_decl(P, D_FN, start);
        d->is_test = true;
        d->test_trap = -1;
        Token *nm = expect(P, TK_STRING);
        d->test_name = nm->kind == TK_STRING ? intern(nm->str.bytes, nm->str.len) : intern_cstr("?");
        d->name = intern_cstr(arena_printf("test.%d", P->ntests++));
        d->name_span = nm->span;
        if (pk(P) == TK_IDENT && !strcmp(peek(P)->name, "expect")) {
            advance(P);
            Token *tr = peek(P);
            if (tr->kind == TK_IDENT && !strcmp(tr->name, "trap")) advance(P);
            else syntax_error(P, "'trap' after 'expect'");
            d->test_trap = -2;
            if (accept(P, TK_LPAREN)) {
                Token *code = peek(P);
                if (code->kind == TK_IDENT) {
                    advance(P);
                    const char *c = code->name;
                    int v = -1;
                    if (c[0] == 'T' && strlen(c) == 5 && isdigit((unsigned char)c[1]) && isdigit((unsigned char)c[2]) &&
                        isdigit((unsigned char)c[3]) && isdigit((unsigned char)c[4]))
                        v = atoi(c + 1);
                    if (v < 0 || v > 13)
                        diag_error(code->span, "E0311", "unknown trap code '%s' (expected T0000 ... T0013)", c);
                    else d->test_trap = v;
                } else {
                    syntax_error(P, "trap code such as T0001");
                }
                expect(P, TK_RPAREN);
            }
        }
        TypeExpr *v = NEW(TypeExpr);
        v->kind = TE_PRIM;
        v->prim = PRIM_VOID;
        v->span = start;
        d->ret_te = NEW(TypeExpr);
        d->ret_te->kind = TE_ERRUNION;
        d->ret_te->elem = v;
        d->ret_te->span = start;
        d->body = parse_block(P);
        d->span = sp_from(P, start);
        vec_push(P->mod->decls, d);
        return;
    }
    if (t->kind == KW_EXPORT) {
        advance(P);
        if (pk(P) != KW_FN && !at_inline(P)) {
            syntax_error(P, "'fn' after 'export'");
            sync_stmt(P);
            return;
        }
        vec_push(P->mod->decls, parse_fn_inline(P, start, false, true));
        return;
    }
    bool is_pub = accept(P, KW_PUB);
    if (pk(P) == KW_EXTERN) {
        // ExternDecl ::= "pub"? "extern" STRING_LIT STRING_LIT? "{" ExternFn* "}"
        advance(P);
        Token *abi = expect(P, TK_STRING);
        Sym abi_s = abi->kind == TK_STRING ? intern(abi->str.bytes, abi->str.len) : intern_cstr("C");
        Sym lib = NULL;
        if (pk(P) == TK_STRING) {
            Token *l = advance(P);
            lib = intern(l->str.bytes, l->str.len);
        }
        expect(P, TK_LBRACE);
        while (pk(P) == KW_FN) {
            Span fs = advance(P)->span;
            Decl *d = new_decl(P, D_HOST_FN, fs);
            d->extern_c = true;
            d->is_pub = is_pub;
            d->abi = abi_s;
            d->abi_span = abi->span;
            d->link_lib = lib;
            d->name = expect_ident(P, &d->name_span);
            parse_params(P, d);
            d->ret_te = parse_type(P);
            expect(P, TK_SEMI);
            d->span = sp_from(P, fs);
            vec_push(P->mod->decls, d);
        }
        expect(P, TK_RBRACE);
        return;
    }
    // `threadlocal` is a keyword only here, before a module-level `let`
    bool tls = pk(P) == TK_IDENT && !strcmp(peek(P)->name, "threadlocal") && peek2(P)->kind == KW_LET;
    if (tls) advance(P);
    if (at_inline(P)) {
        vec_push(P->mod->decls, parse_fn_inline(P, start, is_pub, false));
        return;
    }
    switch (pk(P)) {
    case KW_FN: vec_push(P->mod->decls, parse_fn(P, start, is_pub, false)); return;
    case KW_STRUCT: {
        advance(P);
        Decl *d = new_decl(P, D_STRUCT, start);
        d->is_pub = is_pub;
        d->name = expect_ident(P, &d->name_span);
        parse_generic_params(P, d);
        expect(P, TK_LBRACE);
        parse_field_list(P, &d->fields);
        parse_methods(P, d);
        expect(P, TK_RBRACE);
        d->span = sp_from(P, start);
        vec_push(P->mod->decls, d);
        return;
    }
    case KW_UNION: {
        advance(P);
        Decl *d = new_decl(P, D_UNION, start);
        d->is_pub = is_pub;
        d->name = expect_ident(P, &d->name_span);
        parse_generic_params(P, d);
        expect(P, TK_LBRACE);
        while (pk(P) == TK_IDENT && !at_inline(P)) {
            Variant v = {0};
            v.name = expect_ident(P, &v.span);
            if (accept(P, TK_COLON)) v.te = parse_type(P);
            vec_push(d->variants, v);
            if (!accept(P, TK_COMMA)) break;
        }
        parse_methods(P, d);
        expect(P, TK_RBRACE);
        d->span = sp_from(P, start);
        vec_push(P->mod->decls, d);
        return;
    }
    case KW_ENUM: {
        advance(P);
        Decl *d = new_decl(P, D_ENUM, start);
        d->is_pub = is_pub;
        d->name = expect_ident(P, &d->name_span);
        if (accept(P, TK_COLON)) d->te = parse_type(P);
        expect(P, TK_LBRACE);
        while (pk(P) == TK_IDENT && !at_inline(P)) {
            Variant v = {0};
            v.name = expect_ident(P, &v.span);
            if (accept(P, TK_EQ)) {
                Span vs = peek(P)->span;
                bool neg = accept(P, TK_MINUS);
                Token *n = peek(P);
                if (n->kind == TK_INT || n->kind == TK_CHAR) {
                    advance(P);
                    v.has_value = true;
                    v.value = neg ? -(i128)n->ival : (i128)n->ival;
                } else {
                    syntax_error(P, "integer literal");
                }
                v.value_span = sp_from(P, vs);
            }
            vec_push(d->variants, v);
            if (!accept(P, TK_COMMA)) break;
        }
        parse_methods(P, d);
        expect(P, TK_RBRACE);
        d->span = sp_from(P, start);
        vec_push(P->mod->decls, d);
        return;
    }
    case KW_CONST: {
        advance(P);
        Decl *d = new_decl(P, D_CONST, start);
        d->is_pub = is_pub;
        d->name = expect_ident(P, &d->name_span);
        expect(P, TK_COLON);
        d->te = parse_type(P);
        expect(P, TK_EQ);
        d->init = parse_expr(P);
        expect(P, TK_SEMI);
        d->span = sp_from(P, start);
        vec_push(P->mod->decls, d);
        return;
    }
    case KW_LET: {
        advance(P);
        Decl *d = new_decl(P, D_GLOBAL, start);
        d->is_pub = is_pub;
        d->tls = tls;
        d->mut = accept(P, KW_MUT);
        d->name = expect_ident(P, &d->name_span);
        if (!accept(P, TK_COLON)) {
            syntax_error(P, "':' (global variables need an explicit type)");
        } else {
            d->te = parse_type(P);
        }
        expect(P, TK_EQ);
        d->init = parse_expr(P);
        expect(P, TK_SEMI);
        d->span = sp_from(P, start);
        vec_push(P->mod->decls, d);
        return;
    }
    default:
        syntax_error(P, "declaration");
        sync_stmt(P);
        return;
    }
}

Module *parse_module(SourceFile *f) {
    TokenVec toks = lex_file(f);
    Parser P = {.f = f, .t = toks.data, .n = toks.len, .errors_at_pos = -1};
    P.mod = NEW(Module);
    P.mod->file = f;
    while (pk(&P) != TK_EOF) {
        int before = P.pos;
        parse_top(&P);
        if (P.pos == before) advance(&P);
    }
    return P.mod;
}
