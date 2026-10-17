// lexer.c - tokenizer (maximal munch, whole file up front).
#include "lexer.h"

#include "diag.h"

const char *prim_names[PRIM_COUNT] = {
#define X(id, s) s,
    PRIMS(X)
#undef X
};

static const char *kw_names[] = {
#define X(id, s) s,
    KEYWORDS(X)
#undef X
};

static const char *reserved_future[] = {"async", "await", "comptime", "impl", "interface",
                                        "null", "trait", "type", "unsafe", "yield"};

const char *tok_kind_name(TokKind k) {
    static const char *punct[] = {
        "+", "-", "*", "/", "%", "+%", "-%", "*%", "&", "|", "^", "~", "<<", ">>",
        "==", "!=", "<", "<=", ">", ">=", "=", "+=", "-=", "*=", "/=", "%=", "+%=", "-%=", "*%=",
        "&=", "|=", "^=", "<<=", ">>=", "!", ".", "..", "...", ",", ":", ";", "=>",
        "(", ")", "{", "}", "[", "]",
    };
    switch (k) {
    case TK_EOF: return "end of file";
    case TK_IDENT: return "identifier";
    case TK_INT: return "integer literal";
    case TK_FLOAT: return "float literal";
    case TK_STRING: return "string literal";
    case TK_CHAR: return "character literal";
    case TK_BUILTIN: return "builtin";
    case TK_DISCARD: return "_";
    case TK_PRIM: return "type name";
    default: break;
    }
    if (k >= KW_AND && k <= KW_WHILE) return kw_names[k - KW_AND];
    if (k >= TK_PLUS && k <= TK_RBRACKET) return punct[k - TK_PLUS];
    return "?";
}

typedef struct Lexer {
    SourceFile *f;
    const char *s;
    uint32_t pos, len;
    TokenVec toks;
} Lexer;

static Span mkspan(Lexer *L, uint32_t a, uint32_t b) { return (Span){L->f->id, a, b}; }

static bool is_ident_start(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
static bool is_ident_char(char c) { return is_ident_start(c) || (c >= '0' && c <= '9'); }
static int hexval(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static void push(Lexer *L, Token t) { vec_push(L->toks, t); }

// Skip whitespace and comments. Returns false at EOF.
static void skip_ws(Lexer *L) {
    for (;;) {
        if (L->pos >= L->len) return;
        char c = L->s[L->pos];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            L->pos++;
        } else if (c == '/' && L->pos + 1 < L->len && L->s[L->pos + 1] == '/') {
            while (L->pos < L->len && L->s[L->pos] != '\n') L->pos++;
        } else {
            return;
        }
    }
}

// Decode one escape sequence starting at the backslash; appends UTF-8 bytes.
static void lex_escape(Lexer *L, CharVec *out, bool ascii_only) {
    uint32_t start = L->pos;
    L->pos++;  // backslash
    if (L->pos >= L->len) {
        diag_error(mkspan(L, start, L->pos), "E0006", "unterminated escape sequence");
        return;
    }
    char c = L->s[L->pos++];
    switch (c) {
    case 'n': vec_push(*out, '\n'); return;
    case 'r': vec_push(*out, '\r'); return;
    case 't': vec_push(*out, '\t'); return;
    case '0': vec_push(*out, '\0'); return;
    case '\\': vec_push(*out, '\\'); return;
    case '"': vec_push(*out, '"'); return;
    case '\'': vec_push(*out, '\''); return;
    case 'x': {
        int a = L->pos < L->len ? hexval(L->s[L->pos]) : -1;
        int b = L->pos + 1 < L->len ? hexval(L->s[L->pos + 1]) : -1;
        if (a < 0 || b < 0) {
            diag_error(mkspan(L, start, L->pos), "E0006", "\\x escape needs two hex digits");
            return;
        }
        L->pos += 2;
        vec_push(*out, (char)(a * 16 + b));
        return;
    }
    case 'u': {
        if (L->pos >= L->len || L->s[L->pos] != '{') {
            diag_error(mkspan(L, start, L->pos), "E0006", "\\u escape must be written \\u{HEX}");
            return;
        }
        L->pos++;
        uint32_t cp = 0;
        int nd = 0;
        while (L->pos < L->len && hexval(L->s[L->pos]) >= 0) {
            cp = cp * 16 + (uint32_t)hexval(L->s[L->pos++]);
            nd++;
            if (nd > 6) break;
        }
        if (L->pos >= L->len || L->s[L->pos] != '}' || nd == 0 || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) {
            diag_error(mkspan(L, start, L->pos), "E0006", "invalid \\u{...} escape");
            return;
        }
        L->pos++;
        if (ascii_only && cp > 0x7F) {
            diag_error(mkspan(L, start, L->pos), "E0004", "character literal must be ASCII");
            return;
        }
        if (cp < 0x80) {
            vec_push(*out, (char)cp);
        } else if (cp < 0x800) {
            vec_push(*out, (char)(0xC0 | (cp >> 6)));
            vec_push(*out, (char)(0x80 | (cp & 0x3F)));
        } else if (cp < 0x10000) {
            vec_push(*out, (char)(0xE0 | (cp >> 12)));
            vec_push(*out, (char)(0x80 | ((cp >> 6) & 0x3F)));
            vec_push(*out, (char)(0x80 | (cp & 0x3F)));
        } else {
            vec_push(*out, (char)(0xF0 | (cp >> 18)));
            vec_push(*out, (char)(0x80 | ((cp >> 12) & 0x3F)));
            vec_push(*out, (char)(0x80 | ((cp >> 6) & 0x3F)));
            vec_push(*out, (char)(0x80 | (cp & 0x3F)));
        }
        return;
    }
    default:
        diag_error(mkspan(L, start, L->pos), "E0006", "unknown escape sequence '\\%c'", c);
        return;
    }
}

static void lex_number(Lexer *L) {
    uint32_t start = L->pos;
    const char *s = L->s;
    int base = 10;
    if (s[L->pos] == '0' && L->pos + 1 < L->len && (s[L->pos + 1] == 'x' || s[L->pos + 1] == 'b' || s[L->pos + 1] == 'o')) {
        base = s[L->pos + 1] == 'x' ? 16 : s[L->pos + 1] == 'b' ? 2 : 8;
        L->pos += 2;
    }
    char buf[128];
    int bl = 0;
    bool is_float = false, bad = false;
    for (;;) {
        if (L->pos >= L->len) break;
        char c = s[L->pos];
        if (c == '_') {
            L->pos++;
            continue;
        }
        int v = hexval(c);
        if (base == 16 ? v >= 0 : (c >= '0' && c <= '9')) {
            if (base == 2 && c > '1') bad = true;
            if (base == 8 && c > '7') bad = true;
            if (bl < 120) buf[bl++] = c;
            L->pos++;
            continue;
        }
        if (base == 10 && c == '.' && L->pos + 1 < L->len && s[L->pos + 1] >= '0' && s[L->pos + 1] <= '9' && !is_float) {
            is_float = true;
            if (bl < 120) buf[bl++] = c;
            L->pos++;
            continue;
        }
        if (base == 10 && (c == 'e' || c == 'E')) {
            uint32_t p = L->pos + 1;
            if (p < L->len && (s[p] == '+' || s[p] == '-')) p++;
            if (p < L->len && s[p] >= '0' && s[p] <= '9') {
                is_float = true;
                if (bl < 120) buf[bl++] = 'e';
                L->pos++;
                if (s[L->pos] == '+' || s[L->pos] == '-') {
                    if (bl < 120) buf[bl++] = s[L->pos];
                    L->pos++;
                }
                continue;
            }
        }
        break;
    }
    buf[bl] = 0;
    if (L->pos < L->len && is_ident_char(s[L->pos])) {
        while (L->pos < L->len && is_ident_char(s[L->pos])) L->pos++;
        bad = true;
    }
    Token t = {0};
    t.span = mkspan(L, start, L->pos);
    if (bad || bl == 0) {
        diag_error(t.span, "E0005", "malformed number literal");
        t.kind = TK_INT;
        push(L, t);
        return;
    }
    if (is_float) {
        t.kind = TK_FLOAT;
        t.fval = strtod(buf, NULL);
        t.float_text = arena_strndup(buf, (size_t)bl);
    } else {
        t.kind = TK_INT;
        uint64_t v = 0;
        bool ovf = false;
        for (int i = 0; i < bl; i++) {
            uint64_t d = (uint64_t)hexval(buf[i]);
            if (v > (UINT64_MAX - d) / (uint64_t)base) ovf = true;
            v = v * (uint64_t)base + d;
        }
        if (ovf) diag_error(t.span, "E0005", "integer literal does not fit in 64 bits");
        t.ival = v;
    }
    push(L, t);
}

TokenVec lex_file(SourceFile *f) {
    Lexer L = {.f = f, .s = f->text, .len = (uint32_t)f->len};
    if (L.len >= 3 && (unsigned char)L.s[0] == 0xEF && (unsigned char)L.s[1] == 0xBB && (unsigned char)L.s[2] == 0xBF) {
        diag_error(mkspan(&L, 0, 3), "E0001", "source file must not start with a byte-order mark");
        L.pos = 3;
    }
    for (;;) {
        skip_ws(&L);
        if (L.pos >= L.len) break;
        uint32_t start = L.pos;
        char c = L.s[L.pos];
        if (is_ident_start(c)) {
            while (L.pos < L.len && is_ident_char(L.s[L.pos])) L.pos++;
            size_t n = L.pos - start;
            Token t = {.span = mkspan(&L, start, L.pos)};
            const char *w = L.s + start;
            if (n == 1 && w[0] == '_') {
                t.kind = TK_DISCARD;
                push(&L, t);
                continue;
            }
            t.kind = TK_IDENT;
            for (int i = 0; i < (int)(sizeof kw_names / sizeof *kw_names); i++) {
                if (strlen(kw_names[i]) == n && memcmp(kw_names[i], w, n) == 0) {
                    t.kind = (TokKind)(KW_AND + i);
                    break;
                }
            }
            if (t.kind == TK_IDENT) {
                for (int i = 0; i < PRIM_COUNT; i++) {
                    if (strlen(prim_names[i]) == n && memcmp(prim_names[i], w, n) == 0) {
                        t.kind = TK_PRIM;
                        t.prim = (PrimKind)i;
                        break;
                    }
                }
            }
            if (t.kind == TK_IDENT) {
                for (size_t i = 0; i < sizeof reserved_future / sizeof *reserved_future; i++) {
                    if (strlen(reserved_future[i]) == n && memcmp(reserved_future[i], w, n) == 0) {
                        diag_error(t.span, "E0003", "'%s' is reserved for future use", reserved_future[i]);
                    }
                }
                t.name = intern(w, n);
            }
            push(&L, t);
            continue;
        }
        if (c >= '0' && c <= '9') {
            lex_number(&L);
            continue;
        }
        if (c == '"') {
            L.pos++;
            CharVec buf = {0};
            bool closed = false;
            while (L.pos < L.len) {
                char d = L.s[L.pos];
                if (d == '"') {
                    L.pos++;
                    closed = true;
                    break;
                }
                if (d == '\n') break;
                if (d == '\\') {
                    lex_escape(&L, &buf, false);
                    continue;
                }
                vec_push(buf, d);
                L.pos++;
            }
            Token t = {.kind = TK_STRING, .span = mkspan(&L, start, L.pos)};
            if (!closed) diag_error(t.span, "E0006", "unterminated string literal");
            t.str.bytes = arena_strndup(buf.data ? buf.data : "", (size_t)buf.len);
            t.str.len = (uint32_t)buf.len;
            free(buf.data);
            push(&L, t);
            continue;
        }
        if (c == '\'') {
            L.pos++;
            CharVec buf = {0};
            if (L.pos < L.len && L.s[L.pos] == '\\') {
                lex_escape(&L, &buf, true);
            } else if (L.pos < L.len && L.s[L.pos] != '\'') {
                if ((unsigned char)L.s[L.pos] >= 0x80) {
                    diag_error(mkspan(&L, start, L.pos + 1), "E0004", "character literal must be ASCII");
                    while (L.pos < L.len && (unsigned char)L.s[L.pos] >= 0x80) L.pos++;
                } else {
                    vec_push(buf, L.s[L.pos]);
                    L.pos++;
                }
            }
            Token t = {.kind = TK_CHAR};
            if (L.pos < L.len && L.s[L.pos] == '\'') {
                L.pos++;
            } else {
                diag_error(mkspan(&L, start, L.pos), "E0004", "malformed character literal");
            }
            t.span = mkspan(&L, start, L.pos);
            t.ival = buf.len ? (unsigned char)buf.data[0] : 0;
            free(buf.data);
            push(&L, t);
            continue;
        }
        if (c == '@') {
            L.pos++;
            while (L.pos < L.len && is_ident_char(L.s[L.pos])) L.pos++;
            Token t = {.kind = TK_BUILTIN, .span = mkspan(&L, start, L.pos)};
            if (L.pos == start + 1) diag_error(t.span, "E0002", "expected builtin name after '@'");
            t.name = intern(L.s + start + 1, L.pos - start - 1);
            push(&L, t);
            continue;
        }
        // operators: maximal munch
        static const struct {
            const char *s;
            TokKind k;
        } ops[] = {
            {"...", TK_ELLIPSIS},    {"+%=", TK_PLUS_PCT_EQ}, {"-%=", TK_MINUS_PCT_EQ}, {"*%=", TK_STAR_PCT_EQ}, {"<<=", TK_SHL_EQ},
            {">>=", TK_SHR_EQ},      {"+%", TK_PLUS_PCT},      {"-%", TK_MINUS_PCT},    {"*%", TK_STAR_PCT},
            {"<<", TK_SHL},          {">>", TK_SHR},           {"==", TK_EQEQ},         {"!=", TK_NE},
            {"<=", TK_LE},           {">=", TK_GE},            {"+=", TK_PLUS_EQ},      {"-=", TK_MINUS_EQ},
            {"*=", TK_STAR_EQ},      {"/=", TK_SLASH_EQ},      {"%=", TK_PERCENT_EQ},   {"&=", TK_AMP_EQ},
            {"|=", TK_PIPE_EQ},      {"^=", TK_CARET_EQ},      {"..", TK_DOTDOT},       {"=>", TK_FATARROW},
            {"+", TK_PLUS},           {"-", TK_MINUS},         {"*", TK_STAR},
            {"/", TK_SLASH},         {"%", TK_PERCENT},        {"&", TK_AMP},           {"|", TK_PIPE},
            {"^", TK_CARET},         {"~", TK_TILDE},          {"<", TK_LT},            {">", TK_GT},
            {"=", TK_EQ},            {"!", TK_BANG},           {".", TK_DOT},           {",", TK_COMMA},
            {":", TK_COLON},         {";", TK_SEMI},           {"(", TK_LPAREN},        {")", TK_RPAREN},
            {"{", TK_LBRACE},        {"}", TK_RBRACE},         {"[", TK_LBRACKET},      {"]", TK_RBRACKET},
        };
        bool matched = false;
        for (size_t i = 0; i < sizeof ops / sizeof *ops; i++) {
            size_t n = strlen(ops[i].s);
            if (L.pos + n <= L.len && memcmp(L.s + L.pos, ops[i].s, n) == 0) {
                L.pos += (uint32_t)n;
                Token t = {.kind = ops[i].k, .span = mkspan(&L, start, L.pos)};
                push(&L, t);
                matched = true;
                break;
            }
        }
        if (!matched) {
            uint32_t e = L.pos + 1;
            while (e < L.len && ((unsigned char)L.s[e] & 0xC0) == 0x80) e++;
            diag_error(mkspan(&L, start, e), "E0002", "unexpected character");
            L.pos = e;
        }
    }
    push(&L, (Token){.kind = TK_EOF, .span = mkspan(&L, L.len, L.len)});
    return L.toks;
}
