// types.c - type construction, interning, substitution, printing, layout.
#include "types.h"

#include "diag.h"

Target g_target = {32, true, "wasm32-unknown-unknown"};

Type *t_invalid, *t_void, *t_bool, *t_error, *t_never, *t_untyped_int, *t_untyped_float;
Type *t_u8, *t_u16, *t_u32, *t_u64, *t_usize, *t_i8, *t_i16, *t_i32, *t_i64, *t_isize, *t_f32, *t_f64;
Type *t_v128;

static int next_type_id = 1;

// ---------------------------------------------------------------------------
// Interning: a hash table keyed on the structural fields.
// ---------------------------------------------------------------------------
static Type **ttab;
static size_t ttab_cap, ttab_len;

static uint64_t mix(uint64_t h, uint64_t v) { return (h ^ v) * 0x100000001b3ull + (h >> 29); }

static uint64_t type_hash(const Type *t) {
    uint64_t h = 0xcbf29ce484222325ull;
    h = mix(h, (uint64_t)t->kind);
    h = mix(h, (uint64_t)t->bits);
    h = mix(h, (uint64_t)t->is_signed | ((uint64_t)t->is_size << 1) | ((uint64_t)t->mut << 2));
    h = mix(h, (uint64_t)t->lanes);
    h = mix(h, (uint64_t)(uintptr_t)t->elem);
    h = mix(h, (uint64_t)t->len);
    h = mix(h, (uint64_t)(uintptr_t)t->len_param);
    h = mix(h, (uint64_t)(uintptr_t)t->ret);
    h = mix(h, (uint64_t)(uintptr_t)t->decl);
    h = mix(h, (uint64_t)t->cval);
    for (int i = 0; i < t->nparams; i++) h = mix(h, (uint64_t)(uintptr_t)t->params[i]);
    for (int i = 0; i < t->nargs; i++) h = mix(h, (uint64_t)(uintptr_t)t->args[i]);
    return h;
}

static bool type_eq_struct(const Type *a, const Type *b) {
    if (a->kind != b->kind || a->bits != b->bits || a->is_signed != b->is_signed || a->is_size != b->is_size ||
        a->mut != b->mut || a->lanes != b->lanes || a->elem != b->elem || a->len != b->len ||
        a->len_param != b->len_param || a->ret != b->ret || a->decl != b->decl || a->cval != b->cval ||
        a->nparams != b->nparams || a->nargs != b->nargs)
        return false;
    for (int i = 0; i < a->nparams; i++)
        if (a->params[i] != b->params[i]) return false;
    for (int i = 0; i < a->nargs; i++)
        if (a->args[i] != b->args[i]) return false;
    return true;
}

static Type *intern_type(Type *proto) {
    if (ttab_len * 2 >= ttab_cap) {
        size_t ncap = ttab_cap ? ttab_cap * 2 : 1024;
        Type **nt = calloc(ncap, sizeof *nt);
        for (size_t i = 0; i < ttab_cap; i++) {
            if (!ttab[i]) continue;
            size_t j = type_hash(ttab[i]) & (ncap - 1);
            while (nt[j]) j = (j + 1) & (ncap - 1);
            nt[j] = ttab[i];
        }
        free(ttab);
        ttab = nt;
        ttab_cap = ncap;
    }
    uint64_t h = type_hash(proto);
    size_t j = h & (ttab_cap - 1);
    while (ttab[j]) {
        if (type_eq_struct(ttab[j], proto)) return ttab[j];
        j = (j + 1) & (ttab_cap - 1);
    }
    Type *t = NEW(Type);
    *t = *proto;
    if (proto->nparams) {
        t->params = NEWN(Type *, proto->nparams);
        memcpy(t->params, proto->params, sizeof(Type *) * (size_t)proto->nparams);
    }
    if (proto->nargs) {
        t->args = NEWN(Type *, proto->nargs);
        memcpy(t->args, proto->args, sizeof(Type *) * (size_t)proto->nargs);
    }
    t->id = next_type_id++;
    // has_params
    bool hp = t->kind == TY_PARAM;
    if (t->elem && t->elem->has_params) hp = true;
    if (t->len_param) hp = true;
    if (t->ret && t->ret->has_params) hp = true;
    for (int i = 0; i < t->nparams; i++)
        if (t->params[i]->has_params) hp = true;
    for (int i = 0; i < t->nargs; i++)
        if (t->args[i]->has_params) hp = true;
    t->has_params = hp;
    bool he = t->kind == TY_ENUM;
    if (t->elem && t->elem->has_enum) he = true;
    if (t->ret && t->ret->has_enum) he = true;
    for (int i = 0; i < t->nparams; i++)
        if (t->params[i]->has_enum) he = true;
    for (int i = 0; i < t->nargs; i++)
        if (t->args[i]->has_enum) he = true;
    t->has_enum = he;
    ttab[j] = t;
    ttab_len++;
    return t;
}

static Type *simple(TypeKind k) {
    Type p = {.kind = k};
    return intern_type(&p);
}

Type *type_int(int bits, bool is_signed, bool is_size) {
    Type p = {.kind = TY_INT, .bits = bits, .is_signed = is_signed, .is_size = is_size};
    return intern_type(&p);
}
Type *type_float(int bits) {
    Type p = {.kind = TY_FLOAT, .bits = bits};
    return intern_type(&p);
}
Type *type_vec(Type *lane, int lanes) {
    Type p = {.kind = TY_VEC, .elem = lane, .lanes = lanes, .bits = lane->bits, .is_signed = lane->is_signed};
    return intern_type(&p);
}
Type *type_ptr(Type *elem, bool mut) {
    Type p = {.kind = TY_PTR, .elem = elem, .mut = mut};
    return intern_type(&p);
}
Type *type_slice(Type *elem, bool mut) {
    Type p = {.kind = TY_SLICE, .elem = elem, .mut = mut};
    return intern_type(&p);
}
Type *type_array(Type *elem, int64_t len) {
    Type p = {.kind = TY_ARRAY, .elem = elem, .len = len};
    return intern_type(&p);
}
Type *type_array_param(Type *elem, Type *len_param) {
    Type p = {.kind = TY_ARRAY, .elem = elem, .len = -1, .len_param = len_param};
    return intern_type(&p);
}
Type *type_fn(Type **params, int n, Type *ret) {
    Type p = {.kind = TY_FN, .params = params, .nparams = n, .ret = ret};
    return intern_type(&p);
}
Type *type_named(Decl *d, Type **args, int n) {
    Type p = {.kind = d->kind == D_UNION ? TY_UNION : TY_STRUCT, .decl = d, .args = args, .nargs = n};
    return intern_type(&p);
}
Type *type_enum(Decl *d, Type *repr) {
    Type p = {.kind = TY_ENUM, .decl = d, .elem = repr};
    return intern_type(&p);
}
Type *type_errunion(Type *elem) { return type_errunion_set(elem, NULL); }
// set!T for the error set declared by set (NULL: any error).
Type *type_errunion_set(Type *elem, Decl *set) {
    Type p = {.kind = TY_ERRUNION, .elem = elem, .decl = set};
    return intern_type(&p);
}
// The error type of the set declared by set (NULL: error, any error).
Type *type_errset(Decl *set) {
    if (!set) return t_error;
    Type p = {.kind = TY_ERROR, .decl = set};
    return intern_type(&p);
}
Type *type_constint(int64_t v) {
    Type p = {.kind = TY_CONSTINT, .cval = v};
    return intern_type(&p);
}

Type *type_param(Decl *owner, int index, Sym name, Constraint c, bool is_const, Type *const_type) {
    Type *t = NEW(Type);
    t->kind = TY_PARAM;
    t->id = next_type_id++;
    t->owner = owner;
    t->pindex = index;
    t->name = name;
    t->constraint = c;
    t->is_const = is_const;
    t->const_type = const_type;
    t->has_params = true;
    return t;
}

void types_init(void) {
    t_invalid = simple(TY_INVALID);
    t_void = simple(TY_VOID);
    t_bool = simple(TY_BOOL);
    t_error = simple(TY_ERROR);
    t_never = simple(TY_NEVER);
    t_untyped_int = simple(TY_UNTYPED_INT);
    t_untyped_float = simple(TY_UNTYPED_FLOAT);
    t_u8 = type_int(8, false, false);
    t_u16 = type_int(16, false, false);
    t_u32 = type_int(32, false, false);
    t_u64 = type_int(64, false, false);
    t_usize = type_int(g_target.ptr_bits, false, true);
    t_i8 = type_int(8, true, false);
    t_i16 = type_int(16, true, false);
    t_i32 = type_int(32, true, false);
    t_i64 = type_int(64, true, false);
    t_isize = type_int(g_target.ptr_bits, true, true);
    t_f32 = type_float(32);
    t_f64 = type_float(64);
    t_v128 = type_vraw(128);
}

// v128, v256, v512: raw bits without lanes (TY_V128 of that width).
Type *type_vraw(int bits) {
    Type p = {.kind = TY_V128, .bits = bits};
    return intern_type(&p);
}

int simd_bits(const Type *t) { return t->kind == TY_VEC ? t->lanes * t->elem->bits : t->bits; }

Type *type_prim(PrimKind p) {
    switch (p) {
    case PRIM_U8: return t_u8;
    case PRIM_U16: return t_u16;
    case PRIM_U32: return t_u32;
    case PRIM_U64: return t_u64;
    case PRIM_USIZE: return t_usize;
    case PRIM_I8: return t_i8;
    case PRIM_I16: return t_i16;
    case PRIM_I32: return t_i32;
    case PRIM_I64: return t_i64;
    case PRIM_ISIZE: return t_isize;
    case PRIM_F32: return t_f32;
    case PRIM_F64: return t_f64;
    case PRIM_BOOL: return t_bool;
    case PRIM_VOID: return t_void;
    case PRIM_I8X16: return type_vec(t_i8, 16);
    case PRIM_U8X16: return type_vec(t_u8, 16);
    case PRIM_I16X8: return type_vec(t_i16, 8);
    case PRIM_U16X8: return type_vec(t_u16, 8);
    case PRIM_I32X4: return type_vec(t_i32, 4);
    case PRIM_U32X4: return type_vec(t_u32, 4);
    case PRIM_I64X2: return type_vec(t_i64, 2);
    case PRIM_U64X2: return type_vec(t_u64, 2);
    case PRIM_F32X4: return type_vec(t_f32, 4);
    case PRIM_F64X2: return type_vec(t_f64, 2);
    case PRIM_V128: return t_v128;
    default: break;
    }
    // the wider vectors: the 128-bit ones' lane types, with 2x or 4x the lanes
    if (p >= PRIM_I8X32 && p <= PRIM_V512) {
        int k = p - PRIM_I8X32, scale = 2;
        if (p >= PRIM_I8X64) {
            k = p - PRIM_I8X64;
            scale = 4;
        }
        Type *narrow = type_prim((PrimKind)(PRIM_I8X16 + k));
        if (narrow->kind == TY_V128) return type_vraw(128 * scale);
        return type_vec(narrow->elem, narrow->lanes * scale);
    }
    return t_invalid;
}

// Anonymous struct decls are interned by (field names, field types).
static Vec(Decl *) anon_decls;

Decl *anon_struct_decl(Module *m, FieldVec fields, Span sp) {
    for (int i = 0; i < anon_decls.len; i++) {
        Decl *d = anon_decls.data[i];
        if (d->fields.len != fields.len) continue;
        bool same = true;
        for (int j = 0; j < fields.len && same; j++)
            if (d->fields.data[j].name != fields.data[j].name || d->fields.data[j].type != fields.data[j].type)
                same = false;
        if (same) return d;
    }
    Decl *d = NEW(Decl);
    d->kind = D_STRUCT;
    d->anon = true;
    d->span = sp;
    d->module = m;
    d->name = intern_cstr("struct");
    d->fields = fields;
    d->state = 2;
    d->type = type_named(d, NULL, 0);
    vec_push(anon_decls, d);
    return d;
}

bool ty_is_aggregate(Type *t) {
    switch (t->kind) {
    case TY_STRUCT: case TY_UNION: case TY_ARRAY: case TY_SLICE: case TY_ERRUNION: return true;
    default: return false;
    }
}

// ---------------------------------------------------------------------------
// Substitution
// ---------------------------------------------------------------------------
Type *type_subst(Type *t, Type **from, Type **to, int n) {
    if (!t || !t->has_params || n == 0) return t;
    switch (t->kind) {
    case TY_PARAM:
        for (int i = 0; i < n; i++)
            if (from[i] == t) return to[i];
        return t;
    case TY_PTR: return type_ptr(type_subst(t->elem, from, to, n), t->mut);
    case TY_SLICE: return type_slice(type_subst(t->elem, from, to, n), t->mut);
    case TY_ERRUNION: return type_errunion_set(type_subst(t->elem, from, to, n), t->decl);
    case TY_ARRAY: {
        Type *e = type_subst(t->elem, from, to, n);
        if (t->len_param) {
            Type *l = type_subst(t->len_param, from, to, n);
            if (l->kind == TY_CONSTINT) return type_array(e, l->cval);
            if (l->kind == TY_PARAM) return type_array_param(e, l);
            return type_array(e, 0);
        }
        return type_array(e, t->len);
    }
    case TY_FN: {
        Type **ps = NEWN(Type *, t->nparams + 1);
        for (int i = 0; i < t->nparams; i++) ps[i] = type_subst(t->params[i], from, to, n);
        return type_fn(ps, t->nparams, type_subst(t->ret, from, to, n));
    }
    case TY_STRUCT:
    case TY_UNION: {
        Type **as = NEWN(Type *, t->nargs + 1);
        for (int i = 0; i < t->nargs; i++) as[i] = type_subst(t->args[i], from, to, n);
        return type_named(t->decl, as, t->nargs);
    }
    default: return t;
    }
}

Type *type_lower_enums(Type *t) {
    if (!t || !t->has_enum) return t;
    switch (t->kind) {
    case TY_ENUM: return t->elem;
    case TY_PTR: return type_ptr(type_lower_enums(t->elem), t->mut);
    case TY_SLICE: return type_slice(type_lower_enums(t->elem), t->mut);
    case TY_ERRUNION: return type_errunion_set(type_lower_enums(t->elem), t->decl);
    case TY_ARRAY:
        if (t->len_param) return type_array_param(type_lower_enums(t->elem), t->len_param);
        return type_array(type_lower_enums(t->elem), t->len);
    case TY_FN: {
        Type **ps = NEWN(Type *, t->nparams + 1);
        for (int i = 0; i < t->nparams; i++) ps[i] = type_lower_enums(t->params[i]);
        return type_fn(ps, t->nparams, type_lower_enums(t->ret));
    }
    case TY_STRUCT:
    case TY_UNION: {
        Type **as = NEWN(Type *, t->nargs + 1);
        for (int i = 0; i < t->nargs; i++) as[i] = type_lower_enums(t->args[i]);
        return type_named(t->decl, as, t->nargs);
    }
    default: return t;
    }
}

static Type *inst_member(Type *owner, Type *member) {
    Decl *d = owner->decl;
    if (!d->gparams.len || !member->has_params) return member;
    Type **from = NEWN(Type *, d->gparams.len);
    for (int i = 0; i < d->gparams.len; i++) from[i] = d->gparams.data[i].type;
    return type_subst(member, from, owner->args, owner->nargs);
}

Type *type_field(Type *st, int i) { return inst_member(st, st->decl->fields.data[i].type); }
Type *type_variant(Type *ut, int i) {
    Type *p = ut->decl->variants.data[i].type;
    return p ? inst_member(ut, p) : NULL;
}

// ---------------------------------------------------------------------------
// Printing (canonical source syntax)
// ---------------------------------------------------------------------------
static void tstr(CharVec *b, Type *t);

static void puts_b(CharVec *b, const char *s) {
    while (*s) vec_push(*b, *s++);
}

static void tstr(CharVec *b, Type *t) {
    char tmp[64];
    switch (t->kind) {
    case TY_INVALID: puts_b(b, "<invalid>"); return;
    case TY_VOID: puts_b(b, "void"); return;
    case TY_BOOL: puts_b(b, "bool"); return;
    case TY_INT:
        if (t->is_size) puts_b(b, t->is_signed ? "isize" : "usize");
        else {
            snprintf(tmp, sizeof tmp, "%c%d", t->is_signed ? 'i' : 'u', t->bits);
            puts_b(b, tmp);
        }
        return;
    case TY_FLOAT: snprintf(tmp, sizeof tmp, "f%d", t->bits); puts_b(b, tmp); return;
    case TY_VEC:
        snprintf(tmp, sizeof tmp, "%c%dx%d", t->elem->kind == TY_FLOAT ? 'f' : t->elem->is_signed ? 'i' : 'u',
                 t->elem->bits, t->lanes);
        puts_b(b, tmp);
        return;
    case TY_V128: snprintf(tmp, sizeof tmp, "v%d", t->bits); puts_b(b, tmp); return;
    case TY_PTR: puts_b(b, t->mut ? "*mut " : "*"); tstr(b, t->elem); return;
    case TY_SLICE: puts_b(b, t->mut ? "[]mut " : "[]"); tstr(b, t->elem); return;
    case TY_ARRAY:
        if (t->len_param) {
            puts_b(b, "[");
            puts_b(b, t->len_param->name);
            puts_b(b, "]");
        } else {
            snprintf(tmp, sizeof tmp, "[%lld]", (long long)t->len);
            puts_b(b, tmp);
        }
        tstr(b, t->elem);
        return;
    case TY_FN:
        puts_b(b, "fn(");
        for (int i = 0; i < t->nparams; i++) {
            if (i) puts_b(b, ", ");
            tstr(b, t->params[i]);
        }
        puts_b(b, ") ");
        tstr(b, t->ret);
        return;
    case TY_STRUCT:
    case TY_UNION:
        if (t->decl->anon) {
            puts_b(b, "struct { ");
            for (int i = 0; i < t->decl->fields.len; i++) {
                if (i) puts_b(b, ", ");
                puts_b(b, t->decl->fields.data[i].name);
                puts_b(b, ": ");
                tstr(b, t->decl->fields.data[i].type);
            }
            puts_b(b, " }");
            return;
        }
        puts_b(b, t->decl->name);
        if (t->nargs) {
            puts_b(b, ".[");
            for (int i = 0; i < t->nargs; i++) {
                if (i) puts_b(b, ", ");
                tstr(b, t->args[i]);
            }
            puts_b(b, "]");
        }
        return;
    case TY_ENUM: puts_b(b, t->decl->name); return;
    case TY_ERRUNION:
        if (t->decl) puts_b(b, t->decl->name);
        puts_b(b, "!");
        tstr(b, t->elem);
        return;
    case TY_ERROR: puts_b(b, t->decl ? t->decl->name : "error"); return;
    case TY_PARAM: puts_b(b, t->name); return;
    case TY_CONSTINT: snprintf(tmp, sizeof tmp, "%lld", (long long)t->cval); puts_b(b, tmp); return;
    case TY_UNTYPED_INT: puts_b(b, "integer literal"); return;
    case TY_UNTYPED_FLOAT: puts_b(b, "float literal"); return;
    case TY_NEVER: puts_b(b, "noreturn"); return;
    }
}

const char *type_str(Type *t) {
    CharVec b = {0};
    tstr(&b, t);
    char *s = arena_strndup(b.data ? b.data : "", (size_t)b.len);
    free(b.data);
    return s;
}

// ---------------------------------------------------------------------------
// Layout (spec §5)
// ---------------------------------------------------------------------------
static int64_t round_up(int64_t x, int64_t a) { return a <= 1 ? x : (x + a - 1) / a * a; }

static void layout(Type *t);

int64_t type_size(Type *t) {
    layout(t);
    return t->size;
}
int64_t type_align(Type *t) {
    layout(t);
    return t->align;
}

Type *union_tag_type(Type *ut) { return ut->decl->variants.len <= 256 ? t_u8 : t_u16; }

int64_t union_payload_align(Type *ut) {
    int64_t a = 1;
    for (int i = 0; i < ut->decl->variants.len; i++) {
        Type *p = type_variant(ut, i);
        if (p && type_align(p) > a) a = type_align(p);
    }
    return a;
}

int64_t union_payload_size(Type *ut) {
    int64_t s = 0;
    for (int i = 0; i < ut->decl->variants.len; i++) {
        Type *p = type_variant(ut, i);
        if (p && type_size(p) > s) s = type_size(p);
    }
    return s;
}

int64_t union_payload_offset(Type *ut) { return round_up(type_size(union_tag_type(ut)), union_payload_align(ut)); }

int64_t type_field_offset(Type *st, int i) {
    int64_t off = 0;
    for (int j = 0; j <= i; j++) {
        Type *ft = type_field(st, j);
        off = round_up(off, type_align(ft));
        if (j == i) return off;
        off += type_size(ft);
    }
    return off;
}

static void layout(Type *t) {
    if (t->layout_done) return;
    int64_t ps = g_target.ptr_bits / 8;
    switch (t->kind) {
    case TY_BOOL: t->size = t->align = 1; break;
    case TY_INT:
    case TY_FLOAT: t->size = t->align = t->bits / 8; break;
    case TY_VEC:
    case TY_V128: t->size = t->align = simd_bits(t) / 8; break;
    case TY_PTR:
    case TY_FN: t->size = t->align = ps; break;
    case TY_SLICE: t->size = 2 * ps; t->align = ps; break;
    case TY_ERROR: t->size = t->align = 2; break;
    case TY_ENUM: t->size = t->align = t->elem->bits / 8; break;
    case TY_ARRAY:
        t->align = type_align(t->elem);
        t->size = t->len * type_size(t->elem);
        break;
    case TY_ERRUNION: {
        int64_t a = type_align(t->elem);
        if (a < 2) a = 2;
        t->align = t->elem->kind == TY_VOID ? 2 : a;
        int64_t off = round_up(2, t->elem->kind == TY_VOID ? 1 : type_align(t->elem));
        t->size = round_up(off + (t->elem->kind == TY_VOID ? 0 : type_size(t->elem)), t->align);
        break;
    }
    case TY_STRUCT: {
        int64_t off = 0, al = 1;
        t->layout_done = true;  // guard recursion (infinite-size types are rejected by sema)
        for (int i = 0; i < t->decl->fields.len; i++) {
            Type *ft = type_field(t, i);
            int64_t fa = type_align(ft);
            off = round_up(off, fa) + type_size(ft);
            if (fa > al) al = fa;
        }
        t->align = al;
        t->size = round_up(off, al);
        break;
    }
    case TY_UNION: {
        t->layout_done = true;
        Type *tag = union_tag_type(t);
        int64_t pa = union_payload_align(t);
        int64_t al = pa > tag->align ? pa : type_align(tag);
        t->align = al;
        t->size = round_up(union_payload_offset(t) + union_payload_size(t), al);
        break;
    }
    default: t->size = 0; t->align = 1; break;
    }
    t->layout_done = true;
}

// ---------------------------------------------------------------------------
// Constraints
// ---------------------------------------------------------------------------
Constraint constraint_from_name(Sym s) {
    if (!s) return C_NONE;
    if (!strcmp(s, "Eq")) return C_EQ;
    if (!strcmp(s, "Ord")) return C_ORD;
    if (!strcmp(s, "Num")) return C_NUM;
    if (!strcmp(s, "Int")) return C_INT;
    if (!strcmp(s, "SignedInt")) return C_SIGNED;
    if (!strcmp(s, "UnsignedInt")) return C_UNSIGNED;
    if (!strcmp(s, "Float")) return C_FLOAT;
    if (!strcmp(s, "FloatVector")) return C_FLOAT_VEC;
    return (Constraint)-1;
}

const char *constraint_name(Constraint c) {
    static const char *n[] = {"(none)", "Eq", "Ord", "Num", "Int", "SignedInt", "UnsignedInt", "Float", "FloatVector"};
    return n[c];
}

// Does constraint a imply constraint b?
static bool implies(Constraint a, Constraint b) {
    if (b == C_NONE || a == b) return true;
    switch (a) {
    case C_ORD: return b == C_EQ;
    case C_NUM: return b == C_EQ || b == C_ORD;
    case C_INT: return b == C_EQ || b == C_ORD || b == C_NUM;
    case C_SIGNED:
    case C_UNSIGNED: return b == C_EQ || b == C_ORD || b == C_NUM || b == C_INT;
    case C_FLOAT: return b == C_EQ || b == C_ORD || b == C_NUM;
    default: return false;
    }
}

bool type_satisfies(Type *t, Constraint c) {
    if (t->kind == TY_INVALID) return true;
    if (t->kind == TY_PARAM) return !t->is_const && implies(t->constraint, c);
    switch (c) {
    case C_NONE: return t->kind != TY_VOID && t->kind != TY_CONSTINT && t->kind != TY_NEVER;
    case C_EQ:
        return t->kind == TY_INT || t->kind == TY_FLOAT || t->kind == TY_BOOL || t->kind == TY_PTR ||
               t->kind == TY_ERROR || t->kind == TY_FN || t->kind == TY_ENUM;
    case C_ORD:
    case C_NUM: return t->kind == TY_INT || t->kind == TY_FLOAT;
    case C_INT: return t->kind == TY_INT;
    case C_SIGNED: return t->kind == TY_INT && t->is_signed;
    case C_UNSIGNED: return t->kind == TY_INT && !t->is_signed;
    case C_FLOAT: return t->kind == TY_FLOAT;
    case C_FLOAT_VEC: return ty_is_vec_float(t);
    }
    return false;
}

void int_range(Type *t, i128 *lo, i128 *hi) {
    int b = t->bits;
    if (t->is_signed) {
        *lo = -((i128)1 << (b - 1));
        *hi = ((i128)1 << (b - 1)) - 1;
    } else {
        *lo = 0;
        *hi = ((i128)1 << b) - 1;
    }
}
