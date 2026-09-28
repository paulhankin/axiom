// builtins.h - the closed set of builtin functions. The parser uses the
// signature strings to decide which argument positions are types.
#ifndef PH_BUILTINS_H
#define PH_BUILTINS_H

// Signature: one char per argument: 't' = type, 'e' = expression.
// A trailing '*' means "any number of further expressions".
#define BUILTINS(X)                 \
    X(SIZE_OF, "size_of", "t")      \
    X(ALIGN_OF, "align_of", "t")    \
    X(OFFSET_OF, "offset_of", "te") \
    X(MIN_VALUE, "min_value", "t")  \
    X(MAX_VALUE, "max_value", "t")  \
    X(AS, "as", "te")               \
    X(INT_CAST, "int_cast", "te")   \
    X(TRUNCATE, "truncate", "te")   \
    X(INT_FROM_FLOAT, "int_from_float", "te") \
    X(FLOAT_FROM_INT, "float_from_int", "te") \
    X(FLOAT_CAST, "float_cast", "te")         \
    X(BIT_CAST, "bit_cast", "te")             \
    X(INT_FROM_PTR, "int_from_ptr", "e")      \
    X(PTR_FROM_INT, "ptr_from_int", "te")     \
    X(PTR_CAST, "ptr_cast", "te")             \
    X(SLICE_FROM_PTR, "slice_from_ptr", "ee") \
    X(CAST_SLICE, "cast_slice", "te")         \
    X(MEM_COPY, "mem_copy", "ee")             \
    X(MEM_SET, "mem_set", "ee")               \
    X(MEM_EQ, "mem_eq", "ee")                 \
    X(MEMORY_SIZE, "memory_size", "")         \
    X(MEMORY_GROW, "memory_grow", "e")        \
    X(CLZ, "clz", "e")                        \
    X(CTZ, "ctz", "e")                        \
    X(POPCOUNT, "popcount", "e")              \
    X(ROTL, "rotl", "ee")                     \
    X(ROTR, "rotr", "ee")                     \
    X(ADD_SAT, "add_sat", "ee")               \
    X(SUB_SAT, "sub_sat", "ee")               \
    X(MUL_SAT, "mul_sat", "ee")               \
    X(ADD_CHECKED, "add_checked", "ee")       \
    X(SUB_CHECKED, "sub_checked", "ee")       \
    X(MUL_CHECKED, "mul_checked", "ee")       \
    X(MIN, "min", "ee")                       \
    X(MAX, "max", "ee")                       \
    X(ABS, "abs", "e")                        \
    X(SQRT, "sqrt", "e")                      \
    X(FLOOR, "floor", "e")                    \
    X(CEIL, "ceil", "e")                      \
    X(TRUNC, "trunc", "e")                    \
    X(NEAREST, "nearest", "e")                \
    X(FMA, "fma", "eee")                      \
    X(REM, "rem", "ee")                       \
    X(COPYSIGN, "copysign", "ee")             \
    X(SPLAT, "splat", "e")                    \
    X(VEC, "vec", "t*")                       \
    X(EXTRACT_LANE, "extract_lane", "ee")     \
    X(REPLACE_LANE, "replace_lane", "eee")    \
    X(LOAD, "load", "te")                     \
    X(STORE, "store", "ee")                   \
    X(SHUFFLE, "shuffle", "eee")              \
    X(SWIZZLE, "swizzle", "ee")               \
    X(LANES_EQ, "lanes_eq", "ee")             \
    X(LANES_NE, "lanes_ne", "ee")             \
    X(LANES_LT, "lanes_lt", "ee")             \
    X(LANES_LE, "lanes_le", "ee")             \
    X(LANES_GT, "lanes_gt", "ee")             \
    X(LANES_GE, "lanes_ge", "ee")             \
    X(SELECT, "select", "eee")                \
    X(ANY_TRUE, "any_true", "e")              \
    X(ALL_TRUE, "all_true", "e")              \
    X(BITMASK, "bitmask", "e")                \
    X(REDUCE_ADD, "reduce_add", "e")          \
    X(DOT, "dot", "ee")                       \
    X(TRAP, "trap", "")                       \
    X(ATOMIC_LOAD, "atomic_load", "e")        \
    X(ATOMIC_STORE, "atomic_store", "ee")     \
    X(ATOMIC_ADD, "atomic_add", "ee")         \
    X(ATOMIC_SUB, "atomic_sub", "ee")         \
    X(ATOMIC_AND, "atomic_and", "ee")         \
    X(ATOMIC_OR, "atomic_or", "ee")           \
    X(ATOMIC_XOR, "atomic_xor", "ee")         \
    X(ATOMIC_XCHG, "atomic_xchg", "ee")       \
    X(ATOMIC_MIN, "atomic_min", "ee")         \
    X(ATOMIC_MAX, "atomic_max", "ee")         \
    X(ATOMIC_CAS, "atomic_cas", "eee")        \
    X(ATOMIC_WAIT, "atomic_wait", "eee")      \
    X(ATOMIC_NOTIFY, "atomic_notify", "ee")   \
    X(FENCE, "fence", "")                     \
    X(EXPECT, "expect", "e")                  \
    X(EXPECT_EQ, "expect_eq", "ee")           \
    X(ERROR_NAME, "error_name", "e")          \
    X(MEMORY_DISCARD, "memory_discard", "ee")

typedef enum BuiltinId {
#define X(id, name, sig) BI_##id,
    BUILTINS(X)
#undef X
        BI_COUNT
} BuiltinId;

typedef struct BuiltinInfo {
    const char *name;
    const char *sig;
} BuiltinInfo;

extern const BuiltinInfo builtin_info[BI_COUNT];
int builtin_lookup(const char *name);  // -1 if unknown

#endif
