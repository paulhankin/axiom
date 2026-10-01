// types.h - semantic types. Types are hash-consed: equal types are the same
// pointer, so type equality is pointer equality.
#ifndef AX_TYPES_H
#define AX_TYPES_H

#include "ast.h"

typedef enum TypeKind {
    TY_INVALID,  // error recovery: compatible with everything
    TY_VOID,
    TY_BOOL,
    TY_INT,
    TY_FLOAT,
    TY_VEC,   // lane-typed SIMD vector
    TY_V128,  // raw 128 bits
    TY_PTR,
    TY_SLICE,
    TY_ARRAY,
    TY_FN,
    TY_STRUCT,
    TY_UNION,
    TY_ENUM,   // elem: the representation (an integer type)
    TY_ERRUNION,
    TY_ERROR,
    TY_PARAM,     // generic parameter (type or const)
    TY_CONSTINT,  // generic argument for a const parameter
    TY_UNTYPED_INT,
    TY_UNTYPED_FLOAT,
    TY_NEVER,  // expression that never produces a value (@trap(), return in catch)
} TypeKind;

typedef enum Constraint { C_NONE, C_EQ, C_ORD, C_NUM, C_INT, C_SIGNED, C_UNSIGNED, C_FLOAT } Constraint;

struct Type {
    TypeKind kind;
    int id;
    int bits;        // INT / FLOAT; VEC: lane bits
    bool is_signed;  // INT; VEC with int lanes
    bool is_size;    // usize / isize
    int lanes;       // VEC
    Type *elem;      // PTR, SLICE, ARRAY, ERRUNION, VEC (lane type), ENUM (representation)
    bool mut;        // PTR, SLICE
    int64_t len;     // ARRAY
    Type *len_param; // ARRAY whose length is a const generic parameter
    Type **params;   // FN
    int nparams;
    Type *ret;
    Decl *decl;      // STRUCT, UNION, ENUM
    Type **args;
    int nargs;
    // PARAM
    Decl *owner;
    int pindex;
    Constraint constraint;
    bool is_const;
    Type *const_type;
    Sym name;
    int64_t cval;    // CONSTINT
    bool has_params; // mentions a TY_PARAM somewhere
    bool has_enum;   // mentions a TY_ENUM somewhere
    // layout cache
    int64_t size, align;
    bool layout_done;
    void *backend;   // codegen type cache
};

typedef struct Target {
    int ptr_bits;  // 32 or 64
    bool wasm;
    const char *triple;
} Target;
extern Target g_target;

extern Type *t_invalid, *t_void, *t_bool, *t_error, *t_never, *t_untyped_int, *t_untyped_float;
extern Type *t_u8, *t_u16, *t_u32, *t_u64, *t_usize, *t_i8, *t_i16, *t_i32, *t_i64, *t_isize, *t_f32, *t_f64;
extern Type *t_v128;

void types_init(void);
Type *type_prim(PrimKind p);
Type *type_int(int bits, bool is_signed, bool is_size);
Type *type_float(int bits);
Type *type_vec(Type *lane, int lanes);
Type *type_ptr(Type *elem, bool mut);
Type *type_slice(Type *elem, bool mut);
Type *type_array(Type *elem, int64_t len);
Type *type_array_param(Type *elem, Type *len_param);
Type *type_fn(Type **params, int n, Type *ret);
Type *type_named(Decl *d, Type **args, int n);  // struct or union instance
Type *type_enum(Decl *d, Type *repr);
Type *type_errunion(Type *elem);
Type *type_constint(int64_t v);
Type *type_param(Decl *owner, int index, Sym name, Constraint c, bool is_const, Type *const_type);
Decl *anon_struct_decl(Module *m, FieldVec fields, Span sp);  // interned by field list

// Substitute generic parameters: every TY_PARAM from[i] becomes to[i].
Type *type_subst(Type *t, Type **from, Type **to, int n);
// t with every enum replaced by its representation (for code generation).
Type *type_lower_enums(Type *t);
// Field type of a struct instance / payload type of a union variant instance.
Type *type_field(Type *st, int i);
Type *type_variant(Type *ut, int i);

const char *type_str(Type *t);

// Layout (concrete types only).
int64_t type_size(Type *t);
int64_t type_align(Type *t);
int64_t type_field_offset(Type *st, int i);
Type *union_tag_type(Type *ut);
int64_t union_payload_offset(Type *ut);
int64_t union_payload_align(Type *ut);
int64_t union_payload_size(Type *ut);

// Predicates.
static inline bool ty_is_int(Type *t) { return t->kind == TY_INT; }
static inline bool ty_is_float(Type *t) { return t->kind == TY_FLOAT; }
static inline bool ty_is_num(Type *t) { return t->kind == TY_INT || t->kind == TY_FLOAT; }
static inline bool ty_is_untyped(Type *t) { return t->kind == TY_UNTYPED_INT || t->kind == TY_UNTYPED_FLOAT; }
static inline bool ty_is_vec_float(Type *t) { return t->kind == TY_VEC && t->elem->kind == TY_FLOAT; }
static inline bool ty_is_vec_int(Type *t) { return t->kind == TY_VEC && t->elem->kind == TY_INT; }
static inline bool ty_is_simd(Type *t) { return t->kind == TY_VEC || t->kind == TY_V128; }
bool ty_is_aggregate(Type *t);  // passed by reference in memory: struct, union, array, slice, errunion

// Constraint satisfaction: does type t satisfy c? (t may itself be a TY_PARAM)
bool type_satisfies(Type *t, Constraint c);
Constraint constraint_from_name(Sym s);
const char *constraint_name(Constraint c);

// Integer range of a concrete integer type.
void int_range(Type *t, i128 *lo, i128 *hi);

#endif
