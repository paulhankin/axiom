/* Windows x64: the program database read back through the system's own
   symbol handler (dbghelp.dll, loaded by name at run time), from inside the
   program it describes (run.mjs, "windows"; built with -g at -O0 and -O2):

   - the types of globals (SymFromName, SymGetTypeInfo): a structure's
     members, their names, offsets and types, a bit-field's width and place,
     an array's element type and count, a pointer to the structure itself,
     an enumeration's values, a union, a typedef's type, a function's type;
   - a thread-local's offset in the thread-local template;
   - a function by an address in it (SymFromAddr), and the line of a call
     (SymGetLineFromAddr64);
   - the variables in scope where each probed function calls probe()
     (SymSetContext with the caller's frame, SymEnumSymbols): probe() takes
     its caller's registers from the system's unwinder, reads each variable
     where the symbol handler says it is (a register, or memory at a
     register plus an offset) and compares the values with those the caller
     gave: at -O0 every variable must be found and right; in optimized code
     every one that has a location must be right, and some must have one.

   An Axiom function (symbols.ax, built into the same program) is probed
   too. It prints "ok" and the number of checks, or what failed. */
/* windows-only */
/* x86-64-only */
#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef void *HANDLE;
void *LoadLibraryA(const char *name);
void *GetProcAddress(void *mod, const char *name);
void *GetModuleHandleW(const unsigned short *name);
HANDLE GetCurrentProcess(void);
void RtlCaptureContext(void *ctx);
void *RtlLookupFunctionEntry(unsigned long long pc, unsigned long long *base, void *history);
void *RtlVirtualUnwind(unsigned long type, unsigned long long base, unsigned long long pc, void *entry, void *context,
                       void **data, unsigned long long *frame, void *pointers);

/* dbghelp's structures */
typedef struct {
    unsigned long SizeOfStruct, TypeIndex;
    unsigned long long Reserved[2];
    unsigned long Index, Size;
    unsigned long long ModBase;
    unsigned long Flags, pad;
    unsigned long long Value, Address;
    unsigned long Register, Scope, Tag, NameLen, MaxNameLen;
    char Name[256];
} SYMINFO;

typedef struct {
    unsigned long long InstructionOffset, ReturnOffset, FrameOffset, StackOffset, BackingStoreOffset, FuncTableEntry;
    unsigned long long Params[4], Reserved[5];
    int Virtual;
    unsigned long Reserved2;
} STACKFRAME;

typedef struct {
    unsigned long SizeOfStruct;
    void *Key;
    unsigned long LineNumber;
    char *FileName;
    unsigned long long Address;
} LINE64;

enum { TAG_FUNCTION = 5, TAG_DATA = 7, TAG_UDT = 11, TAG_ENUM = 12, TAG_FUNCTION_TYPE = 13, TAG_POINTER = 14,
       TAG_ARRAY = 15, TAG_BASE = 16 };
enum { TI_SYMTAG, TI_SYMNAME, TI_LENGTH, TI_TYPE, TI_TYPEID, TI_BASETYPE, TI_ARRAYINDEXTYPEID, TI_FINDCHILDREN,
       TI_DATAKIND, TI_ADDRESSOFFSET, TI_OFFSET, TI_VALUE, TI_COUNT, TI_CHILDRENCOUNT, TI_BITPOSITION };
#define SYMFLAG_REGISTER 0x8
#define SYMFLAG_REGREL 0x10
#define SYMFLAG_FRAMEREL 0x20
#define SYMFLAG_PARAMETER 0x40
#define SYMFLAG_TLSREL 0x4000

static int (*pSymInitialize)(HANDLE, const char *, int);
static unsigned long (*pSymSetOptions)(unsigned long);
static int (*pSymFromAddr)(HANDLE, unsigned long long, unsigned long long *, SYMINFO *);
static int (*pSymFromName)(HANDLE, const char *, SYMINFO *);
static int (*pSymSetContext)(HANDLE, STACKFRAME *, void *);
static int (*pSymEnumSymbols)(HANDLE, unsigned long long, const char *, int (*)(SYMINFO *, unsigned long, void *),
                              void *);
static int (*pSymGetTypeInfo)(HANDLE, unsigned long long, unsigned long, int, void *);
static int (*pSymGetLineFromAddr64)(HANDLE, unsigned long long, unsigned long *, LINE64 *);

static HANDLE proc;
static unsigned long long modbase;
static long checks, failures;

static void check(int ok, const char *what, const char *detail) {
    checks++;
    if (!ok) {
        failures++;
        printf("FAIL %s: %s\n", what, detail);
    }
}

static void checkn(long long got, long long want, const char *what) {
    char b[96];
    snprintf(b, sizeof b, "%lld, not %lld", got, want);
    check(got == want, what, b);
}

/* ---- the types described ---- */
typedef int count_t;
enum color { RED, GREEN = 5, BLUE = -2 };
struct shape {
    int x;
    short y;
    unsigned flags : 3, kind : 9;
    double w;
    struct shape *next;
    char name[8];
    enum color c;
    union { int i; float f; } u;
};
struct shape g_shape = { 1, 2, 5, 300, 2.5, &g_shape, "seven", BLUE, { 9 } };
count_t g_count = 5;
static long long g_table[3][4];
_Thread_local int tl_value = 77;
int (*g_fn)(int, double);

/* ---- type information ---- */
static unsigned long tinfo(unsigned long id, int what) {
    unsigned long v = 0;
    if (!pSymGetTypeInfo(proc, modbase, id, what, &v)) return 0xffffffff;
    return v;
}

static unsigned long long tlen(unsigned long id) {
    unsigned long long v = 0;
    if (!pSymGetTypeInfo(proc, modbase, id, TI_LENGTH, &v)) return ~0ull;
    return v;
}

static int tname(unsigned long id, const char *want) {
    unsigned short *w = 0;
    if (!pSymGetTypeInfo(proc, modbase, id, TI_SYMNAME, &w) || !w) return 0;
    size_t k = 0;
    for (; want[k]; k++) {
        if (w[k] != (unsigned char)want[k]) return 0;
    }
    return w[k] == 0;
}

/* the children of a type (its members, enumerators, parameters) */
static int children(unsigned long id, unsigned long *out, int max) {
    unsigned long n = tinfo(id, TI_CHILDRENCOUNT);
    if (n == 0xffffffff || (int)n > max) return -1;
    unsigned long buf[2 + 32] = { n, 0 };
    if (n && !pSymGetTypeInfo(proc, modbase, id, TI_FINDCHILDREN, buf)) return -1;
    for (unsigned long k = 0; k < n; k++) out[k] = buf[2 + k];
    return (int)n;
}

static long long tvalue(unsigned long id) {
    unsigned char v[24];
    memset(v, 0, sizeof v);
    if (!pSymGetTypeInfo(proc, modbase, id, TI_VALUE, v)) return 0x7fffffff;
    unsigned short vt = (unsigned short)(v[0] | v[1] << 8);
    long long x = 0;
    memcpy(&x, v + 8, 8);
    if (vt == 16) return (signed char)x;  /* VT_I1 */
    if (vt == 2) return (short)x;         /* VT_I2 */
    if (vt == 3 || vt == 22) return (int)x;  /* VT_I4, VT_INT */
    if (vt == 17) return (unsigned char)x;
    if (vt == 18) return (unsigned short)x;
    if (vt == 19 || vt == 23) return (unsigned)x;
    return x;
}

/* a member of structure `st` named `name`: its id (0: none) */
static unsigned long member(unsigned long st, const char *name) {
    unsigned long ch[32];
    int n = children(st, ch, 32);
    for (int k = 0; k < n; k++) {
        if (tname(ch[k], name)) return ch[k];
    }
    return 0;
}

static void check_member(unsigned long st, const char *name, long long off, unsigned long long size, int tag) {
    char what[64];
    snprintf(what, sizeof what, "member %s", name);
    unsigned long m = member(st, name);
    check(m != 0, what, "not found");
    if (!m) return;
    checkn(tinfo(m, TI_SYMTAG), TAG_DATA, what);
    checkn(tinfo(m, TI_OFFSET), off, what);
    unsigned long t = tinfo(m, TI_TYPEID);
    checkn(tlen(t), (long long)size, what);
    checkn(tinfo(t, TI_SYMTAG), tag, what);
}

static void check_types(void) {
    SYMINFO si;
    memset(&si, 0, sizeof si);
    si.SizeOfStruct = 88;
    si.MaxNameLen = 255;
    check(pSymFromName(proc, "g_shape", &si), "g_shape", "not found");
    modbase = si.ModBase;
    checkn((long long)si.Address, (long long)(unsigned long long)&g_shape, "g_shape's address");
    unsigned long st = si.TypeIndex;
    checkn(tinfo(st, TI_SYMTAG), TAG_UDT, "struct shape");
    check(tname(st, "shape"), "struct shape", "its name");
    checkn((long long)tlen(st), sizeof(struct shape), "struct shape's size");
    checkn(tinfo(st, TI_CHILDRENCOUNT), 9, "struct shape's members");
    check_member(st, "x", offsetof(struct shape, x), 4, TAG_BASE);
    check_member(st, "y", offsetof(struct shape, y), 2, TAG_BASE);
    check_member(st, "w", offsetof(struct shape, w), 8, TAG_BASE);
    check_member(st, "next", offsetof(struct shape, next), 8, TAG_POINTER);
    check_member(st, "name", offsetof(struct shape, name), 8, TAG_ARRAY);
    check_member(st, "c", offsetof(struct shape, c), 4, TAG_ENUM);
    check_member(st, "u", offsetof(struct shape, u), 4, TAG_UDT);
    /* bit-fields: their bits from the start of the structure */
    const char *bf[2] = { "flags", "kind" };
    long long bits[2] = { 3, 9 }, at[2] = { 64, 67 };
    for (int k = 0; k < 2; k++) {
        unsigned long m = member(st, bf[k]);
        check(m != 0, bf[k], "not found");
        if (!m) continue;
        checkn((long long)tlen(m), bits[k], bf[k]);
        checkn(8 * (long long)tinfo(m, TI_OFFSET) + tinfo(m, TI_BITPOSITION), at[k], bf[k]);
    }
    /* the pointer: to struct shape itself */
    unsigned long next = tinfo(member(st, "next"), TI_TYPEID);
    unsigned long to = tinfo(next, TI_TYPEID);
    checkn(tinfo(to, TI_SYMTAG), TAG_UDT, "next's target");
    check(tname(to, "shape"), "next's target", "not struct shape");
    checkn((long long)tlen(to), sizeof(struct shape), "next's target's size");
    /* the array: 8 chars */
    unsigned long arr = tinfo(member(st, "name"), TI_TYPEID);
    checkn(tinfo(arr, TI_COUNT), 8, "name's count");
    unsigned long el = tinfo(arr, TI_TYPEID);
    checkn((long long)tlen(el), 1, "name's element");
    checkn(tinfo(el, TI_SYMTAG), TAG_BASE, "name's element");
    /* the enumeration */
    unsigned long en = tinfo(member(st, "c"), TI_TYPEID);
    check(tname(en, "color"), "enum color", "its name");
    unsigned long ch[8];
    int n = children(en, ch, 8);
    checkn(n, 3, "enum color's values");
    const char *names[3] = { "RED", "GREEN", "BLUE" };
    long long vals[3] = { RED, GREEN, BLUE };
    for (int k = 0; k < 3; k++) {
        int found = 0;
        for (int j = 0; j < n; j++) {
            if (tname(ch[j], names[k])) {
                found = 1;
                checkn(tvalue(ch[j]), vals[k], names[k]);
            }
        }
        check(found, names[k], "not found");
    }
    /* the union: two members at 0 */
    unsigned long un = tinfo(member(st, "u"), TI_TYPEID);
    checkn(tinfo(un, TI_CHILDRENCOUNT), 2, "the union's members");
    checkn(tinfo(member(un, "f"), TI_OFFSET), 0, "the union's f");
    /* a typedef's object: an int */
    memset(&si, 0, sizeof si);
    si.SizeOfStruct = 88;
    si.MaxNameLen = 255;
    check(pSymFromName(proc, "g_count", &si), "g_count", "not found");
    checkn(tinfo(si.TypeIndex, TI_SYMTAG), TAG_BASE, "g_count's type");
    checkn((long long)tlen(si.TypeIndex), 4, "g_count's type's size");
    /* a two-dimensional array (static) */
    memset(&si, 0, sizeof si);
    si.SizeOfStruct = 88;
    si.MaxNameLen = 255;
    check(pSymFromName(proc, "g_table", &si), "g_table", "not found");
    checkn((long long)si.Address, (long long)(unsigned long long)g_table, "g_table's address");
    checkn((long long)tlen(si.TypeIndex), sizeof g_table, "g_table's size");
    checkn(tinfo(si.TypeIndex, TI_COUNT), 3, "g_table's rows");
    checkn(tinfo(tinfo(si.TypeIndex, TI_TYPEID), TI_COUNT), 4, "g_table's columns");
    /* a pointer to a function: int (int, double) */
    memset(&si, 0, sizeof si);
    si.SizeOfStruct = 88;
    si.MaxNameLen = 255;
    check(pSymFromName(proc, "g_fn", &si), "g_fn", "not found");
    unsigned long ft = tinfo(si.TypeIndex, TI_TYPEID);
    checkn(tinfo(ft, TI_SYMTAG), TAG_FUNCTION_TYPE, "g_fn's target");
    checkn(tinfo(ft, TI_CHILDRENCOUNT), 2, "g_fn's parameters");
    checkn((long long)tlen(tinfo(ft, TI_TYPEID)), 4, "g_fn's result");
    /* the thread-local: its offset in the template (the TLS directory's) */
    memset(&si, 0, sizeof si);
    si.SizeOfStruct = 88;
    si.MaxNameLen = 255;
    check(pSymFromName(proc, "tl_value", &si), "tl_value", "not found");
    check((si.Flags & SYMFLAG_TLSREL) != 0, "tl_value", "not thread-local");
    unsigned char *img = GetModuleHandleW(0);
    unsigned pe = *(unsigned *)(img + 0x3c);
    unsigned tls_rva = *(unsigned *)(img + pe + 24 + 112 + 9 * 8);
    unsigned long long tmpl = *(unsigned long long *)(img + tls_rva);
    int v = 0;
    memcpy(&v, (unsigned char *)tmpl + si.Address, sizeof v);
    checkn(v, 77, "tl_value in the template");
}

/* ---- the variables of a frame ---- */
struct want {
    const char *name;
    unsigned char bytes[64];
    unsigned size;
    int param, seen;
};
static struct want wants[16];
static int nwants, optimized, located, alt_line;
static long values;
static _Alignas(16) unsigned char cctx[0x4d0];

static void want(const char *name, const void *v, unsigned size, int param) {
    struct want *w = &wants[nwants++];
    w->name = name;
    memcpy(w->bytes, v, size);
    w->size = size;
    w->param = param;
    w->seen = 0;
}

/* a register's place in the context (CodeView's numbers: CV_AMD64_*) */
static unsigned char *reg_at(unsigned long r) {
    static const unsigned short gpr[16] = { 0x78, 0x90, 0x80, 0x88, 0xa8, 0xb0, 0xa0, 0x98,
                                            0xb8, 0xc0, 0xc8, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0 };
    if (r >= 328 && r <= 343) return cctx + gpr[r - 328];
    if (r >= 17 && r <= 24) return cctx + gpr[(int[]){ 0, 2, 3, 1, 7, 6, 4, 5 }[r - 17]]; /* eax.. (as rax..) */
    if (r >= 154 && r <= 161) return cctx + 0x1a0 + 16 * (r - 154);
    if (r >= 252 && r <= 259) return cctx + 0x1a0 + 16 * (r - 252 + 8);
    return 0;
}

static int frame_var(SYMINFO *s, unsigned long size, void *data) {
    (void)size;
    (void)data;
    struct want *w = 0;
    for (int k = 0; k < nwants; k++) {
        if (!strcmp(wants[k].name, s->Name)) w = &wants[k];
    }
    if (!w) return 1;
    w->seen = 1;
    char what[64];
    snprintf(what, sizeof what, "variable %s", s->Name);
    if (strcmp(w->name, "v")) checkn(s->Size, w->size, what);  /* (a variable length array's is not known) */
    checkn(!!(s->Flags & SYMFLAG_PARAMETER), w->param, what);
    unsigned char *at = 0;
    /* (a register numbered -1: no location where the frame is) */
    if ((s->Flags & SYMFLAG_REGISTER) && s->Register == 0xffffffff) {
        check(optimized, what, "no location at -O0");
    } else if (s->Flags & SYMFLAG_REGISTER) {
        at = reg_at(s->Register);
        char b[48];
        snprintf(b, sizeof b, "an unknown register %lu", s->Register);
        check(at != 0, what, b);
    } else if (s->Flags & (SYMFLAG_REGREL | SYMFLAG_FRAMEREL)) {
        /* (a register relative location: the register, or the frame's) */
        unsigned char *r = reg_at(s->Register);
        if (s->Flags & SYMFLAG_FRAMEREL) r = cctx + 0xa0;
        char b[48];
        snprintf(b, sizeof b, "an unknown base register %lu", s->Register);
        check(r != 0, what, b);
        if (r) at = (unsigned char *)(*(unsigned long long *)r + s->Address);
    } else if (!optimized) {
        check(0, what, "no location at -O0");
    }
    if (!at) return 1;
    located++;
    values++;
    char b[160];
    int n = snprintf(b, sizeof b, "flags %lx register %lu offset %lld: bytes", s->Flags, s->Register, (long long)s->Address);
    for (unsigned k = 0; k < w->size && n < 150; k++) n += snprintf(b + n, sizeof b - n, " %02x", at[k]);
    check(!memcmp(at, w->bytes, w->size), what, b);
    return 1;
}

/* the function an address is in */
static int in_function(unsigned long long at, const char *name) {
    SYMINFO si;
    memset(&si, 0, sizeof si);
    si.SizeOfStruct = 88;
    si.MaxNameLen = 255;
    unsigned long long disp = 0;
    return pSymFromAddr(proc, at, &disp, &si) && !strcmp(si.Name, name);
}

/* probe_at(line): the variables of the function that called probe() or
   probe_c(), at its call (the frames up to it walked by the system's
   unwinder; those wrappers may be inlined) */
static void probe_at(int line) {
    RtlCaptureContext(cctx);
    for (int k = 0; k < 4; k++) {
        unsigned long long pc = *(unsigned long long *)(cctx + 0xf8), base = 0, frame = 0;
        if (k > 0 && !in_function(pc - 1, "probe") && !in_function(pc - 1, "probe_c")) break;
        void *data = 0;
        void *fe = RtlLookupFunctionEntry(pc, &base, 0);
        RtlVirtualUnwind(0, base, pc, fe, cctx, &data, &frame, 0);
    }
    unsigned long long at = *(unsigned long long *)(cctx + 0xf8) - 1; /* (in the call) */
    LINE64 ln;
    memset(&ln, 0, sizeof ln);
    ln.SizeOfStruct = sizeof ln;
    unsigned long d = 0;
    check(pSymGetLineFromAddr64(proc, at, &d, &ln), "the call's line", "none");
    /* (optimized code: probe_c inlined into its caller, the call is its
       line, in this file) */
    if (!(optimized && alt_line && ln.LineNumber == (unsigned long)alt_line)) checkn(ln.LineNumber, line, "the call's line");
    check(ln.FileName && (strstr(ln.FileName, "symbols.c") || strstr(ln.FileName, "symbols.ax")), "the call's file",
          ln.FileName ? ln.FileName : "none");
    STACKFRAME f;
    memset(&f, 0, sizeof f);
    f.InstructionOffset = at;
    f.FrameOffset = *(unsigned long long *)(cctx + 0xa0);
    f.StackOffset = *(unsigned long long *)(cctx + 0x98);
    pSymSetContext(proc, &f, 0);
    for (int k = 0; k < nwants; k++) wants[k].seen = 0;
    located = 0;
    pSymEnumSymbols(proc, 0, 0, frame_var, 0);
    for (int k = 0; k < nwants; k++) {
        char what[64];
        snprintf(what, sizeof what, "variable %s", wants[k].name);
        check(wants[k].seen || optimized, what, "not in scope");
    }
    check(located > 0, "variables with a location", "none");
    nwants = 0;
}

static volatile int sink;
static void (*volatile probe_at_p)(int) = probe_at;

static void probe(int line) {
    probe_at_p(line);
    sink++;  /* (not a sibling call, which would leave no frame of its own) */
}

/* the probed functions (called through pointers: not inlined) */
static void (*volatile probe_p)(int) = probe;

static int frame0(int a, double b) {
    struct shape p = { a, 7, 3, 9, b, 0, "pq", GREEN, { 4 } };
    count_t local = a * 3;
    unsigned char small = (unsigned char)(a + 200);
    long long big = (long long)a << 40;
    want("a", &a, sizeof a, 1);
    want("b", &b, sizeof b, 1);
    want("p", &p, sizeof p, 0);
    want("local", &local, sizeof local, 0);
    want("small", &small, sizeof small, 0);
    want("big", &big, sizeof big, 0);
    probe_p(__LINE__);
    for (int i = 0; i < 2; i++) {
        int inner = i * 11 + a;
        want("inner", &inner, sizeof inner, 0);
        want("i", &i, sizeof i, 0);
        probe_p(__LINE__);
        sink = inner;
    }
    return p.x + local + small + (int)(big >> 40);
}

/* optimized code: values that live across the call (in callee-saved
   registers or the frame), given to want() by value */
static long long frame1(long long x, int y) {
    long long s = x * 3 + y;
    int t = y * 7 - 1;
    double d = (double)y * 0.25;
    long long w1 = s, w2 = t;
    double w3 = d;
    want("x", &x, sizeof x, 1);
    want("y", &y, sizeof y, 1);
    want("s", &w1, sizeof w1, 0);
    want("t", &(int){ (int)w2 }, sizeof(int), 0);
    want("d", &w3, sizeof w3, 0);
    probe_p(__LINE__);
    sink = (int)(s + t + x + y) + (int)d;
    return s * t + (long long)d + x + y;
}

/* a structure taken apart (its members in registers), a constant, a
   variable length array (held through its address) */
struct pt { int x, y; };
static int frame2(int a) {
    struct pt q = { a * 5, a + 9 };
    int k = 12;
    int n = a + 1;
    int v[n];
    for (int i = 0; i < n; i++) v[i] = i * 3 + a;
    struct pt qc = q;
    want("q", &qc, sizeof qc, 0);
    want("k", &(int){ 12 }, sizeof(int), 0);
    want("a", &a, sizeof a, 1);
    if (optimized) want("v", v, (unsigned)(n * sizeof(int)), 0);  /* (-O0: no location says "at the address in the slot") */
    probe_p(__LINE__);
    sink = q.x + q.y + k + v[n - 1] + qc.x;
    return q.x * q.y + k + v[0];
}

/* the Axiom function's (symbols.ax) access to want() and probe() */
void want_c(const char *name, const void *v, unsigned size, int param) { want(name, v, size, param); }
void probe_c(int line) {
    alt_line = __LINE__ + 1;
    probe_at_p(line);
    alt_line = 0;
    sink++;
}
long long ax_frame(long long n);
static long long (*volatile ax_frame_p)(long long) = ax_frame;

static int (*volatile frame0_p)(int, double) = frame0;
static int (*volatile frame2_p)(int) = frame2;
static long long (*volatile frame1_p)(long long, int) = frame1;

static void get(void *dst, void *m, const char *n) {
    void *f = GetProcAddress(m, n);
    if (!f) printf("FAIL dbghelp.dll has no %s\n", n);
    memcpy(dst, &f, sizeof f);
}

int main(int argc, char **argv) {
    (void)argv;
    optimized = argc > 1;  /* (run.mjs: "optimized" for the -O2 build) */
    void *m = LoadLibraryA("dbghelp.dll");
    if (!m) {
        printf("FAIL no dbghelp.dll\n");
        return 1;
    }
    get(&pSymInitialize, m, "SymInitialize");
    get(&pSymSetOptions, m, "SymSetOptions");
    get(&pSymFromAddr, m, "SymFromAddr");
    get(&pSymFromName, m, "SymFromName");
    get(&pSymSetContext, m, "SymSetContext");
    get(&pSymEnumSymbols, m, "SymEnumSymbols");
    get(&pSymGetTypeInfo, m, "SymGetTypeInfo");
    get(&pSymGetLineFromAddr64, m, "SymGetLineFromAddr64");
    proc = GetCurrentProcess();
    pSymSetOptions(0x10 | 0x2);  /* SYMOPT_LOAD_LINES | SYMOPT_UNDNAME */
    if (!pSymInitialize(proc, 0, 1)) {
        printf("FAIL SymInitialize\n");
        return 1;
    }
    g_fn = frame0;
    sink = g_count + tl_value;  /* (used: data nothing uses is not in the program) */
    check_types();
    /* a function by an address inside it, and its type */
    SYMINFO si;
    memset(&si, 0, sizeof si);
    si.SizeOfStruct = 88;
    si.MaxNameLen = 255;
    unsigned long long disp = 0;
    check(pSymFromAddr(proc, (unsigned long long)frame0 + 4, &disp, &si), "SymFromAddr", "nothing");
    check(!strcmp(si.Name, "frame0"), "SymFromAddr", si.Name);
    checkn((long long)disp, 4, "SymFromAddr's displacement");
    checkn(si.Tag, TAG_FUNCTION, "frame0's tag");
    unsigned long ft = si.TypeIndex;
    checkn(tinfo(ft, TI_SYMTAG), TAG_FUNCTION_TYPE, "frame0's type");
    checkn(tinfo(ft, TI_CHILDRENCOUNT), 2, "frame0's parameters");
    sink = frame0_p(3, 1.5);
    sink = (int)frame1_p(1000000007, 6);
    sink = frame2_p(4);
    sink = (int)ax_frame_p(-5);
    if (failures) {
        printf("%ld of %ld checks failed\n", failures, checks);
        return 1;
    }
    printf("ok %ld checks, %ld values\n", checks, values);
    return 0;
}
