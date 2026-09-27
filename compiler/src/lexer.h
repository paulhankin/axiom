// lexer.h - tokens and tokenizer.
#ifndef PH_LEXER_H
#define PH_LEXER_H

#include "util.h"

#define KEYWORDS(X)          \
    X(AND, "and")            \
    X(AS, "as")              \
    X(BREAK, "break")        \
    X(CATCH, "catch")        \
    X(CONST, "const")        \
    X(CONTINUE, "continue")  \
    X(DEFER, "defer")        \
    X(ELSE, "else")          \
    X(ERROR, "error")        \
    X(EXPORT, "export")      \
    X(EXTERN, "extern")      \
    X(FALSE, "false")        \
    X(FN, "fn")              \
    X(FOR, "for")            \
    X(HOST, "host")          \
    X(IF, "if")              \
    X(IMPORT, "import")      \
    X(IN, "in")              \
    X(LET, "let")            \
    X(LOOP, "loop")          \
    X(MATCH, "match")        \
    X(MUT, "mut")            \
    X(NOT, "not")            \
    X(OR, "or")              \
    X(PUB, "pub")            \
    X(RETURN, "return")      \
    X(SELF, "Self")          \
    X(STRUCT, "struct")      \
    X(TRUE, "true")          \
    X(UNION, "union")        \
    X(WASM, "wasm")          \
    X(WHILE, "while")

// Primitive type names (reserved; lex as TK_PRIM with a PrimKind).
#define PRIMS(X)            \
    X(U8, "u8")             \
    X(U16, "u16")           \
    X(U32, "u32")           \
    X(U64, "u64")           \
    X(USIZE, "usize")       \
    X(I8, "i8")             \
    X(I16, "i16")           \
    X(I32, "i32")           \
    X(I64, "i64")           \
    X(ISIZE, "isize")       \
    X(F32, "f32")           \
    X(F64, "f64")           \
    X(BOOL, "bool")         \
    X(VOID, "void")         \
    X(I8X16, "i8x16")       \
    X(U8X16, "u8x16")       \
    X(I16X8, "i16x8")       \
    X(U16X8, "u16x8")       \
    X(I32X4, "i32x4")       \
    X(U32X4, "u32x4")       \
    X(I64X2, "i64x2")       \
    X(U64X2, "u64x2")       \
    X(F32X4, "f32x4")       \
    X(F64X2, "f64x2")       \
    X(V128, "v128")

typedef enum PrimKind {
#define X(id, s) PRIM_##id,
    PRIMS(X)
#undef X
        PRIM_COUNT
} PrimKind;
extern const char *prim_names[PRIM_COUNT];

typedef enum TokKind {
    TK_EOF,
    TK_IDENT,
    TK_INT,
    TK_FLOAT,
    TK_STRING,
    TK_CHAR,
    TK_BUILTIN,  // @name
    TK_DISCARD,  // _
    TK_PRIM,
#define X(id, s) KW_##id,
    KEYWORDS(X)
#undef X
    // punctuation / operators
    TK_PLUS, TK_MINUS, TK_STAR, TK_SLASH, TK_PERCENT,
    TK_PLUS_PCT, TK_MINUS_PCT, TK_STAR_PCT,
    TK_AMP, TK_PIPE, TK_CARET, TK_TILDE, TK_SHL, TK_SHR,
    TK_EQEQ, TK_NE, TK_LT, TK_LE, TK_GT, TK_GE,
    TK_EQ, TK_PLUS_EQ, TK_MINUS_EQ, TK_STAR_EQ, TK_SLASH_EQ, TK_PERCENT_EQ,
    TK_PLUS_PCT_EQ, TK_MINUS_PCT_EQ, TK_STAR_PCT_EQ,
    TK_AMP_EQ, TK_PIPE_EQ, TK_CARET_EQ, TK_SHL_EQ, TK_SHR_EQ,
    TK_BANG, TK_DOT, TK_DOTDOT, TK_ELLIPSIS, TK_COMMA, TK_COLON, TK_SEMI, TK_FATARROW, TK_ARROW,
    TK_LPAREN, TK_RPAREN, TK_LBRACE, TK_RBRACE, TK_LBRACKET, TK_RBRACKET,
    // inline-wasm mode
    TK_WASM_WORD,   // instruction name or immediate, raw text
    TK_WASM_LOCAL,  // $name
    TK_COUNT
} TokKind;

typedef struct Token {
    TokKind kind;
    Span span;
    union {
        uint64_t ival;  // TK_INT, TK_CHAR
        double fval;    // TK_FLOAT
        Sym name;       // TK_IDENT, TK_BUILTIN, TK_WASM_WORD, TK_WASM_LOCAL
        PrimKind prim;  // TK_PRIM
        struct {
            const char *bytes;
            uint32_t len;
        } str;  // TK_STRING
    };
    const char *float_text;  // TK_FLOAT: literal text without '_' (for exactness checks)
} Token;

const char *tok_kind_name(TokKind k);

typedef Vec(Token) TokenVec;
// Tokenize an entire file. Reports lexical errors through diag.
TokenVec lex_file(SourceFile *f);

#endif
