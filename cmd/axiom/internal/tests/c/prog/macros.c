/* flags: --c-std=c23 */
/* Preprocessor metaprogramming: X-macro tables that generate an
   enumeration, its name strings, a dispatch switch, a structure with
   accessors and its serializer; token pasting into identifiers and
   numbers; stringizing of expanded and unexpanded arguments; argument
   counting; FOR_EACH by a fixed-depth expansion; deferred expansion and
   an EVAL that rescans; __VA_OPT__ for optional commas and suffixes;
   conditional inclusion computed from macros; and macros that expand to
   other macros' names. The program built from them is a small typed
   record store with an opcode interpreter. */
#include "host.h"
#include <stdint.h>
#include <stddef.h>

/* ---- utilities ---- */

#define CAT(a, b) CAT_(a, b)
#define CAT_(a, b) a##b
#define STR(x) STR_(x)
#define STR_(x) #x
#define FIRST(a, ...) a
#define SECOND(a, b, ...) b
#define EMPTY()
#define DEFER(id) id EMPTY()
#define OBSTRUCT(id) id DEFER(EMPTY)()
#define EXPAND(...) __VA_ARGS__

#define COUNT(...) COUNT_(__VA_ARGS__ __VA_OPT__(,) 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define COUNT_(_1, _2, _3, _4, _5, _6, _7, _8, _9, n, ...) n

/* FOR_EACH up to 8 arguments, by counted dispatch */
#define FE_1(m, x) m(x)
#define FE_2(m, x, ...) m(x) FE_1(m, __VA_ARGS__)
#define FE_3(m, x, ...) m(x) FE_2(m, __VA_ARGS__)
#define FE_4(m, x, ...) m(x) FE_3(m, __VA_ARGS__)
#define FE_5(m, x, ...) m(x) FE_4(m, __VA_ARGS__)
#define FE_6(m, x, ...) m(x) FE_5(m, __VA_ARGS__)
#define FE_7(m, x, ...) m(x) FE_6(m, __VA_ARGS__)
#define FE_8(m, x, ...) m(x) FE_7(m, __VA_ARGS__)
#define FOR_EACH(m, ...) CAT(FE_, COUNT(__VA_ARGS__))(m, __VA_ARGS__)

/* EVAL: rescans its argument many times, so that deferred macros expand */
#define EVAL(...) EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL1(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL2(...) __VA_ARGS__

/* a "recursive" repetition through deferral: REPEAT(n, m) calls m(0)..m(n-1) */
#define DEC(n) CAT(DEC_, n)
#define DEC_1 0
#define DEC_2 1
#define DEC_3 2
#define DEC_4 3
#define DEC_5 4
#define DEC_6 5
#define IS_ZERO(n) SECOND_I(CAT(ZERO_PROBE_, n), 0, ~)
#define SECOND_I(...) SECOND(__VA_ARGS__)
#define ZERO_PROBE_0 ~, 1
#define IF(c) CAT(IF_, c)
#define IF_1(t, f) t
#define IF_0(t, f) f
#define REPEAT(n, m) IF(IS_ZERO(n))(, REPEAT_STEP(n, m))
#define REPEAT_STEP(n, m) OBSTRUCT(REPEAT_INDIRECT)()(DEC(n), m) m(DEC(n))
#define REPEAT_INDIRECT() REPEAT

/* ---- an X-macro table of opcodes: name, operand count, effect ---- */

#define OPCODES(X)                                                  \
    X(PUSH, 1, sp[0] = arg;)                                        \
    X(ADD, 0, sp[-2] = sp[-2] + sp[-1];)                            \
    X(SUB, 0, sp[-2] = sp[-2] - sp[-1];)                            \
    X(MUL, 0, sp[-2] = sp[-2] * sp[-1];)                            \
    X(DUP, 0, sp[0] = sp[-1];)                                      \
    X(SWAP, 0, { int64_t t = sp[-1]; sp[-1] = sp[-2]; sp[-2] = t; }) \
    X(NEG, 0, sp[-1] = -sp[-1];)                                    \
    X(SQUARE, 0, sp[-1] = sp[-1] * sp[-1];)

#define AS_ENUM(name, n, ...) OP_##name,
enum opcode { OPCODES(AS_ENUM) OP_COUNT };
#undef AS_ENUM

#define AS_NAME(name, n, ...) [OP_##name] = #name,
static const char *const op_names[] = { OPCODES(AS_NAME) };
#undef AS_NAME

#define AS_ARITY(name, n, ...) n,
static const int op_arity[] = { OPCODES(AS_ARITY) };
#undef AS_ARITY

/* stack effect: +1 for push/dup, -1 for binary, 0 otherwise, from the arity and the name's first letter */
#define DEPTH_PUSH 1
#define DEPTH_DUP 1
#define DEPTH_ADD -1
#define DEPTH_SUB -1
#define DEPTH_MUL -1
#define DEPTH_SWAP 0
#define DEPTH_NEG 0
#define DEPTH_SQUARE 0
#define AS_DEPTH(name, n, ...) [OP_##name] = DEPTH_##name,
static const int op_depth[] = { OPCODES(AS_DEPTH) };
#undef AS_DEPTH

static int64_t interpret(const int64_t *code, int n, int *steps) {
    int64_t stack[64];
    int64_t *sp = stack;
    int pc = 0;
    while (pc < n) {
        enum opcode op = (enum opcode)code[pc++];
        int64_t arg = op_arity[op] ? code[pc++] : 0;
        (void)arg;
        ++*steps;
        switch (op) {
#define AS_CASE(name, n, ...) \
        case OP_##name:       \
            __VA_ARGS__       \
            sp += DEPTH_##name; \
            break;
            OPCODES(AS_CASE)
#undef AS_CASE
        default:
            return -1;
        }
    }
    return sp > stack ? sp[-1] : 0;
}

/* an assembler macro: ASM(PUSH 3) style through pasting */
#define I_PUSH(v) OP_PUSH, v
#define I(op, ...) CAT(I_, op)(__VA_ARGS__)
#define I_ADD() OP_ADD
#define I_SUB() OP_SUB
#define I_MUL() OP_MUL
#define I_DUP() OP_DUP
#define I_SWAP() OP_SWAP
#define I_NEG() OP_NEG
#define I_SQUARE() OP_SQUARE

/* ---- an X-macro structure with typed fields ---- */

#define RECORD_FIELDS(F)          \
    F(int32_t, id, 0)             \
    F(uint8_t, flags, 0x80)       \
    F(int64_t, balance, -1)       \
    F(uint16_t, year, 1970)       \
    F(char, grade, 'C')

#define AS_MEMBER(type, name, def) type name;
struct record { RECORD_FIELDS(AS_MEMBER) };
#undef AS_MEMBER

#define AS_DEFAULT(type, name, def) .name = (def),
static const struct record record_default = { RECORD_FIELDS(AS_DEFAULT) };
#undef AS_DEFAULT

#define AS_ACCESSORS(type, name, def)                                                     \
    static type get_##name(const struct record *r) { return r->name; }                     \
    static void set_##name(struct record *r, type v) { r->name = v; }
RECORD_FIELDS(AS_ACCESSORS)
#undef AS_ACCESSORS

#define AS_HASH(type, name, def) h = (h ^ (uint64_t)(int64_t)r->name) * 1099511628211ull;
static uint64_t record_hash(const struct record *r) {
    uint64_t h = 1469598103934665603ull;
    RECORD_FIELDS(AS_HASH)
    return h;
}
#undef AS_HASH

/* serialization: each field as "name=value;" text */
static int put_text(char *out, const char *s) {
    int n = 0;
    while (s[n]) { out[n] = s[n]; n++; }
    return n;
}
static int put_num(char *out, int64_t v) {
    char t[24];
    int n = 0, k = 0;
    uint64_t u = v < 0 ? 0 - (uint64_t)v : (uint64_t)v;
    do { t[n++] = (char)('0' + u % 10); u /= 10; } while (u);
    if (v < 0) out[k++] = '-';
    while (n) out[k++] = t[--n];
    return k;
}
#define AS_SERIAL(type, name, def) \
    n += put_text(out + n, #name "="); \
    n += put_num(out + n, (int64_t)r->name); \
    out[n++] = ';';
static int serialize(const struct record *r, char *out) {
    int n = 0;
    RECORD_FIELDS(AS_SERIAL)
    return n;
}
#undef AS_SERIAL

#define FIELD_COUNT(type, name, def) +1
enum { NFIELDS = 0 RECORD_FIELDS(FIELD_COUNT) };
#undef FIELD_COUNT

/* ---- __VA_OPT__ ---- */

static int64_t sum_n(int n, ...);
#include <stdarg.h>
static int64_t sum_n(int n, ...) {
    va_list ap;
    va_start(ap, n);
    int64_t s = 0;
    for (int i = 0; i < n; i++) s += va_arg(ap, int);
    va_end(ap);
    return s;
}
#define SUM(...) sum_n(COUNT(__VA_ARGS__) __VA_OPT__(, __VA_ARGS__))
#define TAGGED(name, ...) name##_tag __VA_OPT__(= __VA_ARGS__)
#define LOG_STR(fmt, ...) fmt __VA_OPT__(" with args: " #__VA_ARGS__)

/* ---- pasting numbers and identifiers ---- */

#define HEX(d) 0x##d
#define FLOATNUM(i, f) i##.##f
#define SUFFIXED(n, s) n##s
#define VAR(i) var_##i
#define DECLARE_VARS(x) static int VAR(x) = x * x;
FOR_EACH(DECLARE_VARS, 1, 2, 3, 4, 5)
#define PLUS_VAR(x) +VAR(x)

#define SQUARE_OF(i) +(i) * (i)
#define VERSION_MAJOR 2
#define VERSION_MINOR 7
#define VERSION CAT(CAT(VERSION_MAJOR, 0), VERSION_MINOR)
#if VERSION > 200 && defined(VERSION_MINOR) && !defined(VERSION_PATCH)
#define HAS_FEATURE 1
#else
#define HAS_FEATURE 0
#endif

/* a macro whose expansion is another macro's name */
#define PICK_OP(sign) CAT(OP_FOR_, sign)
#define OP_FOR_plus(a, b) ((a) + (b))
#define OP_FOR_minus(a, b) ((a) - (b))
#define OP_FOR_times(a, b) ((a) * (b))

static void show(const char *s) {
    size_t n = 0;
    while (s[n]) n++;
    print_str(s, n);
}

int main(void) {
    show(STR(CAT(foo, bar)));
    show(STR_(CAT(foo, bar)));
    show(STR(VERSION));
    print_i64(VERSION);
    print_i64(HAS_FEATURE);
    print_i64(COUNT());
    print_i64(COUNT(a));
    print_i64(COUNT(a, b, c, d, e, f, g));
    print_i64(0 FOR_EACH(SQUARE_OF, 1, 2, 3, 4, 5, 6, 7, 8));
    print_i64(0 FOR_EACH(PLUS_VAR, 1, 2, 3, 4, 5));
    print_i64(EVAL(0 REPEAT(6, SQUARE_OF)));
    print_i64(HEX(ff) + HEX(10));
    print_f64(FLOATNUM(3, 25));
    print_u64(SUFFIXED(4294967295, u) + SUFFIXED(1, ull));
    print_i64(PICK_OP(plus)(3, 4) * PICK_OP(times)(2, 5) - PICK_OP(minus)(10, 1));
    print_i64(SUM());
    print_i64(SUM(5));
    print_i64(SUM(1, 2, 3, 4, 5, 6, 7, 8, 9));
    {
        int TAGGED(x, 41);
        int TAGGED(y);
        y_tag = 1;
        print_i64(x_tag + y_tag);
        show(LOG_STR("plain"));
        show(LOG_STR("formatted", a + b, "c"));
    }
    print_i64(OP_COUNT);
    for (int i = 0; i < OP_COUNT; i++) {
        show(op_names[i]);
        print_i64(op_arity[i] * 10 + op_depth[i]);
    }
    {
        static const int64_t prog[] = {
            I(PUSH, 6), I(PUSH, 7), I(MUL), I(DUP), I(SQUARE), I(SWAP), I(SUB), I(NEG), I(PUSH, 1000), I(ADD),
        };
        int steps = 0;
        print_i64(interpret(prog, (int)(sizeof prog / sizeof prog[0]), &steps));
        print_i64(steps);
    }
    {
        struct record r = record_default;
        print_u64(record_hash(&r));
        set_id(&r, 12345);
        set_balance(&r, get_balance(&r) * 1000000007);
        set_flags(&r, (uint8_t)(get_flags(&r) | 3));
        set_year(&r, (uint16_t)(get_year(&r) + 55));
        set_grade(&r, (char)(get_grade(&r) - 2));
        char buf[128];
        int n = serialize(&r, buf);
        print_str(buf, (size_t)n);
        print_u64(record_hash(&r));
        print_i64(NFIELDS);
    }
    return 0;
}

/* expect: foobar */
/* expect: CAT(foo, bar) */
/* expect: 207 */
/* expect: 207 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 7 */
/* expect: 204 */
/* expect: 55 */
/* expect: 55 */
/* expect: 271 */
/* expect: 3.25 */
/* expect: 4294967296 */
/* expect: 61 */
/* expect: 0 */
/* expect: 5 */
/* expect: 45 */
/* expect: 42 */
/* expect: plain */
/* expect: formatted with args: a + b, "c" */
/* expect: 8 */
/* expect: PUSH */
/* expect: 11 */
/* expect: ADD */
/* expect: -1 */
/* expect: SUB */
/* expect: -1 */
/* expect: MUL */
/* expect: -1 */
/* expect: DUP */
/* expect: 1 */
/* expect: SWAP */
/* expect: 0 */
/* expect: NEG */
/* expect: 0 */
/* expect: SQUARE */
/* expect: 0 */
/* expect: -722 */
/* expect: 10 */
/* expect: 11127970707551890619 */
/* expect: id=12345;flags=131;balance=-1000000007;year=2025;grade=65; */
/* expect: 6468507785786060936 */
/* expect: 5 */
