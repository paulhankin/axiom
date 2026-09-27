// =============================================================================
// lexer.ph - tokens and the tokenizer (maximal munch, whole file up front).
// =============================================================================
import "util.ph" as u;
import "diag.ph" as dg;

pub const TK_EOF: i32 = 0;
pub const TK_IDENT: i32 = 1;
pub const TK_INT: i32 = 2;
pub const TK_FLOAT: i32 = 3;
pub const TK_STRING: i32 = 4;
pub const TK_CHAR: i32 = 5;
pub const TK_BUILTIN: i32 = 6;
pub const TK_DISCARD: i32 = 7;
pub const TK_PRIM: i32 = 8;
pub const KW_AND: i32 = 9;
pub const KW_AS: i32 = 10;
pub const KW_BREAK: i32 = 11;
pub const KW_CATCH: i32 = 12;
pub const KW_CONST: i32 = 13;
pub const KW_CONTINUE: i32 = 14;
pub const KW_DEFER: i32 = 15;
pub const KW_ELSE: i32 = 16;
pub const KW_ERROR: i32 = 17;
pub const KW_EXPORT: i32 = 18;
pub const KW_EXTERN: i32 = 19;
pub const KW_FALSE: i32 = 20;
pub const KW_FN: i32 = 21;
pub const KW_FOR: i32 = 22;
pub const KW_HOST: i32 = 23;
pub const KW_IF: i32 = 24;
pub const KW_IMPORT: i32 = 25;
pub const KW_IN: i32 = 26;
pub const KW_LET: i32 = 27;
pub const KW_LOOP: i32 = 28;
pub const KW_MATCH: i32 = 29;
pub const KW_MUT: i32 = 30;
pub const KW_NOT: i32 = 31;
pub const KW_OR: i32 = 32;
pub const KW_PUB: i32 = 33;
pub const KW_RETURN: i32 = 34;
pub const KW_SELF: i32 = 35;
pub const KW_STRUCT: i32 = 36;
pub const KW_TEST: i32 = 37;
pub const KW_TRUE: i32 = 38;
pub const KW_UNION: i32 = 39;
pub const KW_WASM: i32 = 40;
pub const KW_WHILE: i32 = 41;
pub const TK_PLUS: i32 = 42;
pub const TK_MINUS: i32 = 43;
pub const TK_STAR: i32 = 44;
pub const TK_SLASH: i32 = 45;
pub const TK_PERCENT: i32 = 46;
pub const TK_PLUS_PCT: i32 = 47;
pub const TK_MINUS_PCT: i32 = 48;
pub const TK_STAR_PCT: i32 = 49;
pub const TK_AMP: i32 = 50;
pub const TK_PIPE: i32 = 51;
pub const TK_CARET: i32 = 52;
pub const TK_TILDE: i32 = 53;
pub const TK_SHL: i32 = 54;
pub const TK_SHR: i32 = 55;
pub const TK_EQEQ: i32 = 56;
pub const TK_NE: i32 = 57;
pub const TK_LT: i32 = 58;
pub const TK_LE: i32 = 59;
pub const TK_GT: i32 = 60;
pub const TK_GE: i32 = 61;
pub const TK_EQ: i32 = 62;
pub const TK_PLUS_EQ: i32 = 63;
pub const TK_MINUS_EQ: i32 = 64;
pub const TK_STAR_EQ: i32 = 65;
pub const TK_SLASH_EQ: i32 = 66;
pub const TK_PERCENT_EQ: i32 = 67;
pub const TK_PLUS_PCT_EQ: i32 = 68;
pub const TK_MINUS_PCT_EQ: i32 = 69;
pub const TK_STAR_PCT_EQ: i32 = 70;
pub const TK_AMP_EQ: i32 = 71;
pub const TK_PIPE_EQ: i32 = 72;
pub const TK_CARET_EQ: i32 = 73;
pub const TK_SHL_EQ: i32 = 74;
pub const TK_SHR_EQ: i32 = 75;
pub const TK_BANG: i32 = 76;
pub const TK_DOT: i32 = 77;
pub const TK_DOTDOT: i32 = 78;
pub const TK_ELLIPSIS: i32 = 79;
pub const TK_COMMA: i32 = 80;
pub const TK_COLON: i32 = 81;
pub const TK_SEMI: i32 = 82;
pub const TK_FATARROW: i32 = 83;
pub const TK_ARROW: i32 = 84;
pub const TK_LPAREN: i32 = 85;
pub const TK_RPAREN: i32 = 86;
pub const TK_LBRACE: i32 = 87;
pub const TK_RBRACE: i32 = 88;
pub const TK_LBRACKET: i32 = 89;
pub const TK_RBRACKET: i32 = 90;
pub const TK_WASM_WORD: i32 = 91;
pub const TK_WASM_LOCAL: i32 = 92;
pub const TK_COUNT: i32 = 93;
pub const PRIM_U8: i32 = 0;
pub const PRIM_U16: i32 = 1;
pub const PRIM_U32: i32 = 2;
pub const PRIM_U64: i32 = 3;
pub const PRIM_USIZE: i32 = 4;
pub const PRIM_I8: i32 = 5;
pub const PRIM_I16: i32 = 6;
pub const PRIM_I32: i32 = 7;
pub const PRIM_I64: i32 = 8;
pub const PRIM_ISIZE: i32 = 9;
pub const PRIM_F32: i32 = 10;
pub const PRIM_F64: i32 = 11;
pub const PRIM_BOOL: i32 = 12;
pub const PRIM_VOID: i32 = 13;
pub const PRIM_I8X16: i32 = 14;
pub const PRIM_U8X16: i32 = 15;
pub const PRIM_I16X8: i32 = 16;
pub const PRIM_U16X8: i32 = 17;
pub const PRIM_I32X4: i32 = 18;
pub const PRIM_U32X4: i32 = 19;
pub const PRIM_I64X2: i32 = 20;
pub const PRIM_U64X2: i32 = 21;
pub const PRIM_F32X4: i32 = 22;
pub const PRIM_F64X2: i32 = 23;
pub const PRIM_V128: i32 = 24;
pub const PRIM_COUNT: i32 = 25;

pub fn prim_name(i: usize) []u8 {
    let t = ["u8", "u16", "u32", "u64", "usize", "i8", "i16", "i32", "i64", "isize", "f32", "f64", "bool", "void", "i8x16", "u8x16", "i16x8", "u16x8", "i32x4", "u32x4", "i64x2", "u64x2", "f32x4", "f64x2", "v128"];
    return t[i];
}
pub fn kw_name(i: usize) []u8 {
    let t = ["and", "as", "break", "catch", "const", "continue", "defer", "else", "error", "export", "extern", "false", "fn", "for", "host", "if", "import", "in", "let", "loop", "match", "mut", "not", "or", "pub", "return", "Self", "struct", "test", "true", "union", "wasm", "while"];
    return t[i];
}
pub fn punct_name(i: usize) []u8 {
    let t = ["+", "-", "*", "/", "%", "+%", "-%", "*%", "&", "|", "^", "~", "<<", ">>", "==", "!=", "<", "<=", ">", ">=", "=", "+=", "-=", "*=", "/=", "%=", "+%=", "-%=", "*%=", "&=", "|=", "^=", "<<=", ">>=", "!", ".", "..", "...", ",", ":", ";", "=>", "->", "(", ")", "{", "}", "[", "]"];
    return t[i];
}

pub fn tok_kind_name(k: i32) []u8 {
    if k == TK_EOF {
        return "end of file";
    }
    if k == TK_IDENT {
        return "identifier";
    }
    if k == TK_INT {
        return "integer literal";
    }
    if k == TK_FLOAT {
        return "float literal";
    }
    if k == TK_STRING {
        return "string literal";
    }
    if k == TK_CHAR {
        return "character literal";
    }
    if k == TK_BUILTIN {
        return "builtin";
    }
    if k == TK_DISCARD {
        return "_";
    }
    if k == TK_PRIM {
        return "type name";
    }
    if k == TK_WASM_WORD {
        return "wasm instruction";
    }
    if k == TK_WASM_LOCAL {
        return "wasm local";
    }
    if (k >= KW_AND) and (k <= KW_WHILE) {
        return kw_name(@int_cast(usize, k - KW_AND));
    }
    if (k >= TK_PLUS) and (k <= TK_RBRACKET) {
        return punct_name(@int_cast(usize, k - TK_PLUS));
    }
    return "?";
}

pub struct Token {
    kind: i32,
    span: u.Span,
    ival: u64,          // TK_INT, TK_CHAR
    fval: f64,          // TK_FLOAT
    name: []u8,         // TK_IDENT, TK_BUILTIN, TK_WASM_WORD, TK_WASM_LOCAL
    prim: i32,          // TK_PRIM
    str: []u8,          // TK_STRING
    float_text: []u8,   // TK_FLOAT: literal text without '_'
}

fn tok(kind: i32, sp: u.Span) Token {
    return Token.{ kind: kind, span: sp, ival: 0, fval: 0.0, name: u.nosym(), prim: 0, str: u.nosym(),
                   float_text: u.nosym() };
}

struct Lexer {
    f: *mut u.SourceFile,
    s: []u8,
    pos: usize,
    toks: u.List.[Token],
    wasm_pending: bool,  // saw `wasm`, next `{` (not after `struct`) enters wasm mode
}

fn mkspan(L: *Lexer, a: usize, b: usize) u.Span {
    return u.mkspan(L.f.id, @int_cast(u32, a), @int_cast(u32, b));
}

fn is_ident_start(c: u8) bool {
    return ((c >= 'a') and (c <= 'z')) or ((c >= 'A') and (c <= 'Z')) or (c == '_');
}

fn is_ident_char(c: u8) bool {
    return is_ident_start(c) or ((c >= '0') and (c <= '9'));
}

fn hexval(c: u8) i32 {
    if (c >= '0') and (c <= '9') {
        return @as(i32, c - '0');
    }
    if (c >= 'a') and (c <= 'f') {
        return @as(i32, c - 'a') + 10;
    }
    if (c >= 'A') and (c <= 'F') {
        return @as(i32, c - 'A') + 10;
    }
    return -1;
}

fn at(L: *Lexer, i: usize) u8 {
    if i < L.s.len {
        return L.s[i];
    }
    return 0;
}

fn skip_ws(L: *mut Lexer) void {
    loop {
        if L.pos >= L.s.len {
            return;
        }
        let c = L.s[L.pos];
        if (c == ' ') or (c == '\t') or (c == '\n') or (c == '\r') {
            L.pos += 1;
        } else if (c == '/') and ((L.pos + 1) < L.s.len) and (L.s[L.pos + 1] == '/') {
            while (L.pos < L.s.len) and (L.s[L.pos] != '\n') {
                L.pos += 1;
            }
        } else {
            return;
        }
    }
}

// Decodes one escape sequence starting at the backslash; appends UTF-8 bytes.
fn lex_escape(L: *mut Lexer, out: *mut u.Buf, ascii_only: bool) void {
    let start = L.pos;
    L.pos += 1;
    if L.pos >= L.s.len {
        _ = dg.err(mkspan(L, start, L.pos), "E0006", "unterminated escape sequence");
        return;
    }
    let c = L.s[L.pos];
    L.pos += 1;
    if c == 'n' {
        out.c('\n');
        return;
    }
    if c == 'r' {
        out.c('\r');
        return;
    }
    if c == 't' {
        out.c('\t');
        return;
    }
    if c == '0' {
        out.c(0);
        return;
    }
    if c == '\\' {
        out.c('\\');
        return;
    }
    if c == '"' {
        out.c('"');
        return;
    }
    if c == '\'' {
        out.c('\'');
        return;
    }
    if c == 'x' {
        let mut a: i32 = -1;
        let mut b: i32 = -1;
        if L.pos < L.s.len {
            a = hexval(L.s[L.pos]);
        }
        if (L.pos + 1) < L.s.len {
            b = hexval(L.s[L.pos + 1]);
        }
        if (a < 0) or (b < 0) {
            _ = dg.err(mkspan(L, start, L.pos), "E0006", "\\x escape needs two hex digits");
            return;
        }
        L.pos += 2;
        out.c(@int_cast(u8, (a * 16) + b));
        return;
    }
    if c == 'u' {
        if (L.pos >= L.s.len) or (L.s[L.pos] != '{') {
            _ = dg.err(mkspan(L, start, L.pos), "E0006", "\\u escape must be written \\u{HEX}");
            return;
        }
        L.pos += 1;
        let mut cp: u32 = 0;
        let mut nd: i32 = 0;
        while (L.pos < L.s.len) and (hexval(L.s[L.pos]) >= 0) {
            cp = (cp *% 16) +% @int_cast(u32, hexval(L.s[L.pos]));
            L.pos += 1;
            nd += 1;
            if nd > 6 {
                break;
            }
        }
        if (L.pos >= L.s.len) or (L.s[L.pos] != '}') or (nd == 0) or (cp > 0x10FFFF) or ((cp >= 0xD800) and (cp <= 0xDFFF)) {
            _ = dg.err(mkspan(L, start, L.pos), "E0006", "invalid \\u{...} escape");
            return;
        }
        L.pos += 1;
        if ascii_only and (cp > 0x7F) {
            _ = dg.err(mkspan(L, start, L.pos), "E0004", "character literal must be ASCII");
            return;
        }
        if cp < 0x80 {
            out.c(@truncate(u8, cp));
        } else if cp < 0x800 {
            out.c(@truncate(u8, 0xC0 | (cp >> 6)));
            out.c(@truncate(u8, 0x80 | (cp & 0x3F)));
        } else if cp < 0x10000 {
            out.c(@truncate(u8, 0xE0 | (cp >> 12)));
            out.c(@truncate(u8, 0x80 | ((cp >> 6) & 0x3F)));
            out.c(@truncate(u8, 0x80 | (cp & 0x3F)));
        } else {
            out.c(@truncate(u8, 0xF0 | (cp >> 18)));
            out.c(@truncate(u8, 0x80 | ((cp >> 12) & 0x3F)));
            out.c(@truncate(u8, 0x80 | ((cp >> 6) & 0x3F)));
            out.c(@truncate(u8, 0x80 | (cp & 0x3F)));
        }
        return;
    }
    let one = [c];
    _ = dg.err(mkspan(L, start, L.pos), "E0006", u.f1("unknown escape sequence '\\%s'", one[..]));
}

fn lex_number(L: *mut Lexer) void {
    let start = L.pos;
    let s = L.s;
    let mut base: u64 = 10;
    if (s[L.pos] == '0') and ((L.pos + 1) < s.len) and ((s[L.pos + 1] == 'x') or (s[L.pos + 1] == 'b') or (s[L.pos + 1] == 'o')) {
        if s[L.pos + 1] == 'x' {
            base = 16;
        } else if s[L.pos + 1] == 'b' {
            base = 2;
        } else {
            base = 8;
        }
        L.pos += 2;
    }
    let mut buf: [128]u8 = [0; 128];
    let mut bl: usize = 0;
    let mut is_float = false;
    let mut bad = false;
    loop {
        if L.pos >= s.len {
            break;
        }
        let c = s[L.pos];
        if c == '_' {
            L.pos += 1;
            continue;
        }
        let v = hexval(c);
        let mut digit = (c >= '0') and (c <= '9');
        if base == 16 {
            digit = v >= 0;
        }
        if digit {
            if (base == 2) and (c > '1') {
                bad = true;
            }
            if (base == 8) and (c > '7') {
                bad = true;
            }
            if bl < 120 {
                buf[bl] = c;
                bl += 1;
            }
            L.pos += 1;
            continue;
        }
        if (base == 10) and (c == '.') and ((L.pos + 1) < s.len) and (s[L.pos + 1] >= '0') and (s[L.pos + 1] <= '9') and not is_float {
            is_float = true;
            if bl < 120 {
                buf[bl] = c;
                bl += 1;
            }
            L.pos += 1;
            continue;
        }
        if (base == 10) and ((c == 'e') or (c == 'E')) {
            let mut p = L.pos + 1;
            if (p < s.len) and ((s[p] == '+') or (s[p] == '-')) {
                p += 1;
            }
            if (p < s.len) and (s[p] >= '0') and (s[p] <= '9') {
                is_float = true;
                if bl < 120 {
                    buf[bl] = 'e';
                    bl += 1;
                }
                L.pos += 1;
                if (at(L, L.pos) == '+') or (at(L, L.pos) == '-') {
                    if bl < 120 {
                        buf[bl] = s[L.pos];
                        bl += 1;
                    }
                    L.pos += 1;
                }
                continue;
            }
        }
        break;
    }
    if (L.pos < s.len) and is_ident_char(s[L.pos]) {
        while (L.pos < s.len) and is_ident_char(s[L.pos]) {
            L.pos += 1;
        }
        bad = true;
    }
    let mut t = tok(TK_INT, mkspan(L, start, L.pos));
    if bad or (bl == 0) {
        _ = dg.err(t.span, "E0005", "malformed number literal");
        L.toks.push(t);
        return;
    }
    if is_float {
        t.kind = TK_FLOAT;
        t.float_text = u.dup(buf[0..bl]);
        t.fval = u.c_strtod(t.float_text).v;
    } else {
        let mut v: u64 = 0;
        let mut ovf = false;
        for i in 0..bl {
            let d = @int_cast(u64, hexval(buf[i]));
            if v > ((0xFFFF_FFFF_FFFF_FFFF - d) / base) {
                ovf = true;
            }
            v = (v *% base) +% d;
        }
        if ovf {
            _ = dg.err(t.span, "E0005", "integer literal does not fit in 64 bits");
        }
        t.ival = v;
    }
    L.toks.push(t);
}

// Tokens inside `wasm { ... }`.
fn lex_wasm_body(L: *mut Lexer) void {
    loop {
        skip_ws(L);
        if L.pos >= L.s.len {
            _ = dg.err(mkspan(L, L.pos, L.pos), "E0007", "unterminated wasm block");
            return;
        }
        let c = L.s[L.pos];
        let start = L.pos;
        if c == '}' {
            L.pos += 1;
            L.toks.push(tok(TK_RBRACE, mkspan(L, start, L.pos)));
            return;
        }
        if (c == '(') or (c == ')') {
            L.pos += 1;
            let mut k = TK_RPAREN;
            if c == '(' {
                k = TK_LPAREN;
            }
            L.toks.push(tok(k, mkspan(L, start, L.pos)));
            continue;
        }
        if (c == ';') and (at(L, L.pos + 1) == ';') {
            while (L.pos < L.s.len) and (L.s[L.pos] != '\n') {
                L.pos += 1;
            }
            continue;
        }
        if c == '$' {
            L.pos += 1;
            while (L.pos < L.s.len) and is_ident_char(L.s[L.pos]) {
                L.pos += 1;
            }
            let mut t = tok(TK_WASM_LOCAL, mkspan(L, start, L.pos));
            t.name = u.intern(L.s[start + 1..L.pos]);
            L.toks.push(t);
            continue;
        }
        while L.pos < L.s.len {
            let d = L.s[L.pos];
            if (d == ' ') or (d == '\t') or (d == '\n') or (d == '\r') or (d == '(') or (d == ')') or (d == '}') or (d == '$') {
                break;
            }
            if d >= 0x80 {
                break;
            }
            L.pos += 1;
        }
        if L.pos == start {
            _ = dg.err(mkspan(L, start, start + 1), "E0002", "unexpected character in wasm block");
            L.pos += 1;
            continue;
        }
        let mut t = tok(TK_WASM_WORD, mkspan(L, start, L.pos));
        t.name = u.intern(L.s[start..L.pos]);
        L.toks.push(t);
    }
}

// Operators for maximal munch, longest first.
struct OpTok {
    s: []u8,
    k: i32,
}

fn op_table() [49]OpTok {
    return [
        OpTok.{ s: "...", k: TK_ELLIPSIS }, OpTok.{ s: "+%=", k: TK_PLUS_PCT_EQ }, OpTok.{ s: "-%=", k: TK_MINUS_PCT_EQ },
        OpTok.{ s: "*%=", k: TK_STAR_PCT_EQ }, OpTok.{ s: "<<=", k: TK_SHL_EQ }, OpTok.{ s: ">>=", k: TK_SHR_EQ },
        OpTok.{ s: "+%", k: TK_PLUS_PCT }, OpTok.{ s: "-%", k: TK_MINUS_PCT }, OpTok.{ s: "*%", k: TK_STAR_PCT },
        OpTok.{ s: "<<", k: TK_SHL }, OpTok.{ s: ">>", k: TK_SHR }, OpTok.{ s: "==", k: TK_EQEQ },
        OpTok.{ s: "!=", k: TK_NE }, OpTok.{ s: "<=", k: TK_LE }, OpTok.{ s: ">=", k: TK_GE },
        OpTok.{ s: "+=", k: TK_PLUS_EQ }, OpTok.{ s: "-=", k: TK_MINUS_EQ }, OpTok.{ s: "*=", k: TK_STAR_EQ },
        OpTok.{ s: "/=", k: TK_SLASH_EQ }, OpTok.{ s: "%=", k: TK_PERCENT_EQ }, OpTok.{ s: "&=", k: TK_AMP_EQ },
        OpTok.{ s: "|=", k: TK_PIPE_EQ }, OpTok.{ s: "^=", k: TK_CARET_EQ }, OpTok.{ s: "..", k: TK_DOTDOT },
        OpTok.{ s: "=>", k: TK_FATARROW }, OpTok.{ s: "->", k: TK_ARROW }, OpTok.{ s: "+", k: TK_PLUS },
        OpTok.{ s: "-", k: TK_MINUS }, OpTok.{ s: "*", k: TK_STAR }, OpTok.{ s: "/", k: TK_SLASH },
        OpTok.{ s: "%", k: TK_PERCENT }, OpTok.{ s: "&", k: TK_AMP }, OpTok.{ s: "|", k: TK_PIPE },
        OpTok.{ s: "^", k: TK_CARET }, OpTok.{ s: "~", k: TK_TILDE }, OpTok.{ s: "<", k: TK_LT },
        OpTok.{ s: ">", k: TK_GT }, OpTok.{ s: "=", k: TK_EQ }, OpTok.{ s: "!", k: TK_BANG },
        OpTok.{ s: ".", k: TK_DOT }, OpTok.{ s: ",", k: TK_COMMA }, OpTok.{ s: ":", k: TK_COLON },
        OpTok.{ s: ";", k: TK_SEMI }, OpTok.{ s: "(", k: TK_LPAREN }, OpTok.{ s: ")", k: TK_RPAREN },
        OpTok.{ s: "{", k: TK_LBRACE }, OpTok.{ s: "}", k: TK_RBRACE }, OpTok.{ s: "[", k: TK_LBRACKET },
        OpTok.{ s: "]", k: TK_RBRACKET },
    ];
}

fn reserved_future(i: usize) []u8 {
    let t = ["async", "await", "comptime", "enum", "impl", "interface", "null", "trait", "type", "unsafe", "yield"];
    return t[i];
}

// Tokenizes an entire file. Reports lexical errors through diag.
pub fn lex_file(f: *mut u.SourceFile) u.List.[Token] {
    let mut L = Lexer.{ f: f, s: f.text, pos: 0, toks: u.List.[Token].{ data: u.nil.[Token](), len: 0, cap: 0 },
                        wasm_pending: false };
    let ops = op_table();
    if (L.s.len >= 3) and (L.s[0] == 0xEF) and (L.s[1] == 0xBB) and (L.s[2] == 0xBF) {
        _ = dg.err(mkspan(&L, 0, 3), "E0001", "source file must not start with a byte-order mark");
        L.pos = 3;
    }
    loop {
        skip_ws(&mut L);
        if L.pos >= L.s.len {
            break;
        }
        let start = L.pos;
        let c = L.s[L.pos];
        if is_ident_start(c) {
            while (L.pos < L.s.len) and is_ident_char(L.s[L.pos]) {
                L.pos += 1;
            }
            let w = L.s[start..L.pos];
            let mut t = tok(TK_IDENT, mkspan(&L, start, L.pos));
            if (w.len == 1) and (w[0] == '_') {
                t.kind = TK_DISCARD;
                L.toks.push(t);
                continue;
            }
            for i in 0..@as(usize, 33) {
                if u.str_eq(kw_name(i), w) {
                    t.kind = KW_AND + @int_cast(i32, i);
                    break;
                }
            }
            if t.kind == TK_IDENT {
                for i in 0..@int_cast(usize, PRIM_COUNT) {
                    if u.str_eq(prim_name(i), w) {
                        t.kind = TK_PRIM;
                        t.prim = @int_cast(i32, i);
                        break;
                    }
                }
            }
            if t.kind == TK_IDENT {
                for i in 0..@as(usize, 11) {
                    if u.str_eq(reserved_future(i), w) {
                        _ = dg.err(t.span, "E0003", u.f1("'%s' is reserved for future use", reserved_future(i)));
                    }
                }
                t.name = u.intern(w);
            }
            if t.kind == KW_WASM {
                L.wasm_pending = true;
            }
            L.toks.push(t);
            continue;
        }
        if (c >= '0') and (c <= '9') {
            lex_number(&mut L);
            continue;
        }
        if c == '"' {
            L.pos += 1;
            let mut buf = u.Buf.init();
            let mut closed = false;
            while L.pos < L.s.len {
                let d = L.s[L.pos];
                if d == '"' {
                    L.pos += 1;
                    closed = true;
                    break;
                }
                if d == '\n' {
                    break;
                }
                if d == '\\' {
                    lex_escape(&mut L, &mut buf, false);
                    continue;
                }
                buf.c(d);
                L.pos += 1;
            }
            let mut t = tok(TK_STRING, mkspan(&L, start, L.pos));
            if not closed {
                _ = dg.err(t.span, "E0006", "unterminated string literal");
            }
            t.str = buf.done();
            L.toks.push(t);
            continue;
        }
        if c == '\'' {
            L.pos += 1;
            let mut buf = u.Buf.init();
            if (L.pos < L.s.len) and (L.s[L.pos] == '\\') {
                lex_escape(&mut L, &mut buf, true);
            } else if (L.pos < L.s.len) and (L.s[L.pos] != '\'') {
                if L.s[L.pos] >= 0x80 {
                    _ = dg.err(mkspan(&L, start, L.pos + 1), "E0004", "character literal must be ASCII");
                    while (L.pos < L.s.len) and (L.s[L.pos] >= 0x80) {
                        L.pos += 1;
                    }
                } else {
                    buf.c(L.s[L.pos]);
                    L.pos += 1;
                }
            }
            if (L.pos < L.s.len) and (L.s[L.pos] == '\'') {
                L.pos += 1;
            } else {
                _ = dg.err(mkspan(&L, start, L.pos), "E0004", "malformed character literal");
            }
            let mut t = tok(TK_CHAR, mkspan(&L, start, L.pos));
            let bytes = buf.done();
            if bytes.len > 0 {
                t.ival = @as(u64, bytes[0]);
            }
            L.toks.push(t);
            continue;
        }
        if c == '@' {
            L.pos += 1;
            while (L.pos < L.s.len) and is_ident_char(L.s[L.pos]) {
                L.pos += 1;
            }
            let mut t = tok(TK_BUILTIN, mkspan(&L, start, L.pos));
            if L.pos == (start + 1) {
                _ = dg.err(t.span, "E0002", "expected builtin name after '@'");
            }
            t.name = u.intern(L.s[start + 1..L.pos]);
            L.toks.push(t);
            continue;
        }
        // operators: maximal munch
        let mut matched = false;
        for i in @as(usize, 0)..49 {
            let op = ops[i];
            let n = op.s.len;
            if ((L.pos + n) <= L.s.len) and @mem_eq(L.s[L.pos..L.pos + n], op.s) {
                L.pos += n;
                let t = tok(op.k, mkspan(&L, start, L.pos));
                let prev_struct = (L.toks.len > 0) and (L.toks.last().kind == KW_STRUCT);
                L.toks.push(t);
                matched = true;
                if (t.kind == TK_LBRACE) and L.wasm_pending and not prev_struct {
                    L.wasm_pending = false;
                    lex_wasm_body(&mut L);
                } else if (t.kind == TK_SEMI) or (t.kind == TK_RBRACE) {
                    L.wasm_pending = false;
                }
                break;
            }
        }
        if not matched {
            let mut e = L.pos + 1;
            while (e < L.s.len) and ((L.s[e] & 0xC0) == 0x80) {
                e += 1;
            }
            _ = dg.err(mkspan(&L, start, e), "E0002", "unexpected character");
            L.pos = e;
        }
    }
    L.toks.push(tok(TK_EOF, mkspan(&L, L.s.len, L.s.len)));
    return L.toks;
}
