/* flags: --c-std=c89 */
/* A tokenizer for a toy language written as an explicit state machine: one
   big switch on the state inside a loop, with goto between the states of
   the number and string scanners and for error recovery, the classic
   "switch into the middle of a loop" byte copier, keyword recognition by a
   switch on length and first character, and nested comments counted by
   depth. C89: declarations at the start of blocks, no // comments. */
#include "host.h"

enum kind {
    K_EOF, K_IDENT, K_KEYWORD, K_INT, K_FLOAT, K_STRING, K_CHAR, K_OP, K_ERROR
};

static const char *const kind_names[] = {
    "eof", "ident", "keyword", "int", "float", "string", "char", "op", "error"
};

enum kw { KW_NONE, KW_IF, KW_ELSE, KW_WHILE, KW_FN, KW_LET, KW_RETURN, KW_TRUE, KW_FALSE, KW_NIL, KW_LOOP, KW_BREAK };

struct token {
    enum kind kind;
    int start, len;
    int line, col;
    long ival;          /* integer value, wrapped to 32 bits */
    int kw;
};

#define MAXT 1024
static struct token toks[MAXT];
static int ntok;
static int counts[9];
static int errors;

static int keyword(const char *s, int n) {
    switch (n) {
    case 2:
        if (s[0] == 'i' && s[1] == 'f') return KW_IF;
        if (s[0] == 'f' && s[1] == 'n') return KW_FN;
        break;
    case 3:
        switch (s[0]) {
        case 'l': if (s[1] == 'e' && s[2] == 't') return KW_LET; break;
        case 'n': if (s[1] == 'i' && s[2] == 'l') return KW_NIL; break;
        }
        break;
    case 4:
        switch (s[0]) {
        case 'e': if (s[1] == 'l' && s[2] == 's' && s[3] == 'e') return KW_ELSE; break;
        case 't': if (s[1] == 'r' && s[2] == 'u' && s[3] == 'e') return KW_TRUE; break;
        case 'l': if (s[1] == 'o' && s[2] == 'o' && s[3] == 'p') return KW_LOOP; break;
        }
        break;
    case 5:
        if (s[0] == 'w' && s[1] == 'h' && s[2] == 'i' && s[3] == 'l' && s[4] == 'e') return KW_WHILE;
        if (s[0] == 'f' && s[1] == 'a' && s[2] == 'l' && s[3] == 's' && s[4] == 'e') return KW_FALSE;
        if (s[0] == 'b' && s[1] == 'r' && s[2] == 'e' && s[3] == 'a' && s[4] == 'k') return KW_BREAK;
        break;
    case 6:
        if (s[0] == 'r' && s[1] == 'e' && s[2] == 't' && s[3] == 'u' && s[4] == 'r' && s[5] == 'n') return KW_RETURN;
        break;
    default:
        break;
    }
    return KW_NONE;
}

enum state { S_START, S_IDENT, S_ZERO, S_DEC, S_HEX, S_BIN, S_FRAC, S_EXP, S_EXPDIGITS, S_STRING, S_ESC, S_CHAR,
             S_SLASH, S_LINECOMMENT, S_BLOCK, S_BLOCK_STAR, S_BLOCK_SLASH, S_OP };

static int push(enum kind k, int start, int len, int line, int col, long v) {
    struct token *t;
    if (ntok == MAXT) return 0;
    t = &toks[ntok++];
    t->kind = k;
    t->start = start;
    t->len = len;
    t->line = line;
    t->col = col;
    t->ival = v;
    t->kw = 0;
    counts[k]++;
    return 1;
}

static int digit_value(int c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 99;
}

static int max_depth;

static void tokenize(const char *src) {
    enum state st = S_START;
    int i = 0, start = 0, line = 1, col = 1, tline = 1, tcol = 1, depth = 0;
    unsigned long v = 0;
    int c;
    ntok = 0;
    for (;;) {
        c = (unsigned char)src[i];
        switch (st) {
        case S_START:
            start = i;
            tline = line;
            tcol = col;
            v = 0;
            if (c == 0) {
                push(K_EOF, i, 0, line, col, 0);
                return;
            }
            if (c == ' ' || c == '\t' || c == '\r') goto advance;
            if (c == '\n') goto advance;
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') { st = S_IDENT; goto advance; }
            if (c == '0') { st = S_ZERO; goto advance; }
            if (c >= '1' && c <= '9') { v = (unsigned long)(c - '0'); st = S_DEC; goto advance; }
            if (c == '"') { st = S_STRING; goto advance; }
            if (c == '\'') { st = S_CHAR; goto advance; }
            if (c == '/') { st = S_SLASH; goto advance; }
            st = S_OP;
            continue;
        case S_IDENT:
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || (c >= '0' && c <= '9')) goto advance;
            {
                int kw = keyword(src + start, i - start);
                if (push(kw ? K_KEYWORD : K_IDENT, start, i - start, tline, tcol, 0)) toks[ntok - 1].kw = kw;
            }
            st = S_START;
            continue;
        case S_ZERO:
            if (c == 'x' || c == 'X') { st = S_HEX; goto advance; }
            if (c == 'b' || c == 'B') { st = S_BIN; goto advance; }
            if (c == '.') { st = S_FRAC; goto advance; }
            if (c >= '0' && c <= '9') goto bad_number;
            goto int_done;
        case S_DEC:
            if (c >= '0' && c <= '9') {
                v = (v * 10 + (unsigned long)(c - '0')) & 0xffffffffUL;
                goto advance;
            }
            if (c == '.') { st = S_FRAC; goto advance; }
            if (c == 'e' || c == 'E') { st = S_EXP; goto advance; }
            goto int_done;
        case S_HEX:
            if (digit_value(c) < 16) { v = (v * 16 + (unsigned long)digit_value(c)) & 0xffffffffUL; goto advance; }
            if (i - start == 2) goto bad_number;
            goto int_done;
        case S_BIN:
            if (c == '0' || c == '1') { v = (v * 2 + (unsigned long)(c - '0')) & 0xffffffffUL; goto advance; }
            if (c >= '2' && c <= '9') goto bad_number;
            goto int_done;
        case S_FRAC:
            if (c >= '0' && c <= '9') goto advance;
            if (c == 'e' || c == 'E') { st = S_EXP; goto advance; }
            goto float_done;
        case S_EXP:
            if (c == '+' || c == '-') { st = S_EXPDIGITS; goto advance; }
            /* fall through */
        case S_EXPDIGITS:
            if (c >= '0' && c <= '9') { st = S_EXPDIGITS; goto advance; }
            if (src[i - 1] == 'e' || src[i - 1] == 'E' || src[i - 1] == '+' || src[i - 1] == '-') goto bad_number;
            goto float_done;
        case S_STRING:
            if (c == '"') { i++; col++; push(K_STRING, start, i - start, tline, tcol, (long)v); st = S_START; continue; }
            if (c == '\\') { st = S_ESC; goto advance; }
            if (c == '\n' || c == 0) goto unterminated;
            v++;
            goto advance;
        case S_ESC:
            if (c == 0) goto unterminated;
            v++;
            st = S_STRING;
            goto advance;
        case S_CHAR:
            if (c == '\\') {
                if (src[i + 1] == 0) goto unterminated;
                i++;
                col++;
                c = (unsigned char)src[i];
                switch (c) {
                case 'n': c = '\n'; break;
                case 't': c = '\t'; break;
                case '0': c = 0; break;
                default: break;
                }
            }
            if (c == 0 || src[i + 1] != '\'') goto unterminated;
            i += 2;
            col += 2;
            push(K_CHAR, start, i - start, tline, tcol, c);
            st = S_START;
            continue;
        case S_SLASH:
            if (c == '/') { st = S_LINECOMMENT; goto advance; }
            if (c == '*') { st = S_BLOCK; depth = 1; goto advance; }
            push(K_OP, start, 1, tline, tcol, '/');
            st = S_START;
            continue;
        case S_LINECOMMENT:
            if (c == '\n' || c == 0) { st = S_START; continue; }
            goto advance;
        case S_BLOCK:
            if (c == 0) goto unterminated;
            if (c == '*') { st = S_BLOCK_STAR; goto advance; }
            if (c == '/') { st = S_BLOCK_SLASH; goto advance; }
            goto advance;
        case S_BLOCK_STAR:
            if (c == '/') {
                st = --depth ? S_BLOCK : S_START;
                goto advance;
            }
            st = S_BLOCK;
            continue;
        case S_BLOCK_SLASH:
            if (c == '*') {
                if (++depth > max_depth) max_depth = depth;
                st = S_BLOCK;
                goto advance;
            }
            st = S_BLOCK;
            continue;
        case S_OP: {
            /* the longest of the two-character operators */
            static const char two[][3] = { "==", "!=", "<=", ">=", "&&", "||", "->", "<<", ">>", "+=", "-=", "::" };
            int k, len = 1;
            for (k = 0; k < (int)(sizeof two / sizeof two[0]); k++)
                if (two[k][0] == c && two[k][1] == src[i + 1]) { len = 2; break; }
            if (len == 1 && !(c == '+' || c == '-' || c == '*' || c == '%' || c == '<' || c == '>' || c == '=' ||
                              c == '!' || c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' ||
                              c == ',' || c == ';' || c == '.' || c == ':' || c == '&' || c == '|' || c == '^')) {
                push(K_ERROR, start, 1, tline, tcol, c);
                errors++;
            } else {
                push(K_OP, start, len, tline, tcol, c * 256 + (len == 2 ? src[i + 1] : 0));
            }
            i += len;
            col += len;
            st = S_START;
            continue;
        }
        }
        /* not reached: every state continues or jumps */
        errors += 1000;
        return;

    advance:
        if (c == '\n') { line++; col = 1; } else col++;
        i++;
        continue;

    int_done:
        push(K_INT, start, i - start, tline, tcol, (long)v);
        st = S_START;
        continue;

    float_done:
        push(K_FLOAT, start, i - start, tline, tcol, 0);
        st = S_START;
        continue;

    bad_number:
        /* skip the rest of the alphanumeric run */
        while ((src[i] >= '0' && src[i] <= '9') || (src[i] >= 'a' && src[i] <= 'z') || (src[i] >= 'A' && src[i] <= 'Z')) {
            i++;
            col++;
        }
        push(K_ERROR, start, i - start, tline, tcol, 0);
        errors++;
        st = S_START;
        continue;

    unterminated:
        push(K_ERROR, start, i - start, tline, tcol, -1);
        errors++;
        while (src[i] && src[i] != '\n') { i++; col++; }
        st = S_START;
        continue;
    }
}

/* the byte copier that jumps into the middle of its loop */
static void copy8(char *to, const char *from, int count) {
    int n;
    if (count <= 0) return;
    n = (count + 7) / 8;
    switch (count % 8) {
    case 0: do { *to++ = *from++;
    case 7:      *to++ = *from++;
    case 6:      *to++ = *from++;
    case 5:      *to++ = *from++;
    case 4:      *to++ = *from++;
    case 3:      *to++ = *from++;
    case 2:      *to++ = *from++;
    case 1:      *to++ = *from++;
            } while (--n > 0);
    }
}

static const char source[] =
    "fn fib(n) {\n"
    "    if n < 2 { return n; } /* base /* nested */ case */\n"
    "    return fib(n - 1) + fib(n - 2);\n"
    "}\n"
    "let hex = 0xFF_; let bin = 0b1012; let ok = 0b1011 + 0x7fffffff + 4294967297;\n"
    "let f = 3.25e+2 + 1.5 + 0.5e-3 + 6.02E23 + 1e;\n"
    "let s = \"a \\\"quoted\\\" string\\n\"; let c = 'x' + '\\n' + '\\0';\n"
    "while true && !false { loop { break; } } // trailing comment\n"
    "x->y :: z <<= 1 >> 2 != 3 == 4 <= 5 >= 6 || a && b; a += 1; b -= 2; # @ $\n"
    "let bad = \"unterminated\n"
    "let e = 'ab'; let z = 0; let n = nil; let u = 00 ;\n"
    "/* /* /* deep */ */ */ fn_ifelse elsewhere returning\n"
    "1/2/3 /x/ ;\n";

int main(void) {
    int i, sum = 0;
    unsigned long h = 5381;
    char copy[sizeof source];
    tokenize(source);
    print_i64(ntok);
    for (i = 0; i < 9; i++) {
        print_str(kind_names[i], 0 + (i == 0 ? 3 : i == 1 ? 5 : i == 2 ? 7 : i == 3 ? 3 : i == 4 ? 5 : i == 5 ? 6 : i == 6 ? 4 : i == 7 ? 2 : 5));
        print_i64(counts[i]);
    }
    for (i = 0; i < ntok; i++) {
        struct token *t = &toks[i];
        h = (h * 33 + (unsigned long)t->kind * 7 + (unsigned long)t->len + (unsigned long)t->line * 131 +
             (unsigned long)t->col + (unsigned long)t->ival + (unsigned long)t->kw * 17) & 0xffffffffUL;
        if (t->kind == K_INT) sum = (int)((unsigned)sum + (unsigned)t->ival);
    }
    print_u64(h);
    print_i64(sum);
    print_i64(errors);
    print_i64(max_depth);
    {
        /* the last few tokens with their positions */
        for (i = ntok - 5; i < ntok; i++) {
            print_i64(toks[i].line * 1000 + toks[i].col);
            print_str(source + toks[i].start, (size_t)toks[i].len);
        }
    }
    for (i = 0; i <= 20; i++) {
        int k;
        for (k = 0; k < (int)sizeof copy; k++) copy[k] = '-';
        copy8(copy, source + 3, i);
        copy[i] = 0;
        for (k = 0; copy[k]; k++) h = (h * 31 + (unsigned char)copy[k]) & 0xffffffffUL;
    }
    copy8(copy, source, (int)sizeof source);
    print_u64(h);
    print_str(copy + 4, 3);
    return 0;
}

/* expect: 144 */
/* expect: eof */
/* expect: 1 */
/* expect: ident */
/* expect: 28 */
/* expect: keyword */
/* expect: 17 */
/* expect: int */
/* expect: 18 */
/* expect: float */
/* expect: 4 */
/* expect: string */
/* expect: 1 */
/* expect: char */
/* expect: 2 */
/* expect: op */
/* expect: 65 */
/* expect: error */
/* expect: 8 */
/* expect: 2149537511 */
/* expect: -2147483347 */
/* expect: 8 */
/* expect: 3 */
/* expect: 13007 */
/* expect: / */
/* expect: 13008 */
/* expect: x */
/* expect: 13009 */
/* expect: / */
/* expect: 13011 */
/* expect: ; */
/* expect: 14001 */
/* expect:  */
/* expect: 4248300335 */
/* expect: ib( */
