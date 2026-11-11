/* flags: --c-std=c11 */
/* A register virtual machine run three ways that must agree: a switch in a
   loop, a table of handler functions indexed by opcode (each returning the
   next program counter), and a table of handler pointers stored in the
   decoded instructions themselves. Instructions are packed into 32-bit
   words (opcode and three register fields, or a 16-bit immediate) and
   decoded into a struct with bit-fields. The programs are assembled by a
   small assembler with labels and fix-ups: a sieve of primes, a sort,
   a recursive function through a call stack, and checksum loops. */
#include "host.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

enum opcode {
    OP_HALT, OP_LI, OP_MOV, OP_ADD, OP_SUB, OP_MUL, OP_DIVU, OP_REMU, OP_AND, OP_OR, OP_XOR, OP_SHL, OP_SHR,
    OP_ADDI, OP_LD, OP_ST, OP_BEQ, OP_BNE, OP_BLT, OP_JMP, OP_CALL, OP_RET, OP_PUSH, OP_POP, OP_SLT, OP_LUI,
    NOPS
};

typedef struct vm vm;
typedef struct insn insn;
typedef uint32_t (*handler)(vm *, const insn *, uint32_t pc);

struct insn {
    unsigned op : 6;
    unsigned a : 4;
    unsigned b : 4;
    unsigned c : 4;
    signed imm : 16;
    handler fn;
};

#define NREG 16
#define MEMW 4096
struct vm {
    uint32_t r[NREG];
    uint32_t mem[MEMW];
    uint32_t stack[256];
    int sp;
    uint64_t steps;
    bool halted;
};

/* ---- encoding ---- */

static uint32_t enc(enum opcode op, unsigned a, unsigned b, unsigned c) {
    return (uint32_t)op << 26 | (a & 15) << 22 | (b & 15) << 18 | (c & 15) << 14;
}

static uint32_t enc_i(enum opcode op, unsigned a, unsigned b, int imm) {
    return (uint32_t)op << 26 | (a & 15) << 22 | (b & 15) << 18 | ((uint32_t)imm & 0xffff);
}

static insn decode(uint32_t w) {
    insn i = {
        .op = w >> 26,
        .a = w >> 22 & 15,
        .b = w >> 18 & 15,
        .c = w >> 14 & 15,
        .imm = (int16_t)(w & 0xffff),
    };
    return i;
}

/* ---- the switch interpreter ---- */

static void run_switch(vm *m, const uint32_t *code, uint32_t n) {
    uint32_t pc = 0;
    while (pc < n) {
        insn i = decode(code[pc]);
        uint32_t *r = m->r;
        m->steps++;
        pc++;
        switch ((enum opcode)i.op) {
        case OP_HALT: m->halted = true; return;
        case OP_LI: r[i.a] = (uint32_t)(int32_t)i.imm; break;
        case OP_LUI: r[i.a] = (uint32_t)(uint16_t)i.imm << 16 | (r[i.a] & 0xffff); break;
        case OP_MOV: r[i.a] = r[i.b]; break;
        case OP_ADD: r[i.a] = r[i.b] + r[i.c]; break;
        case OP_SUB: r[i.a] = r[i.b] - r[i.c]; break;
        case OP_MUL: r[i.a] = r[i.b] * r[i.c]; break;
        case OP_DIVU: r[i.a] = r[i.c] ? r[i.b] / r[i.c] : 0; break;
        case OP_REMU: r[i.a] = r[i.c] ? r[i.b] % r[i.c] : r[i.b]; break;
        case OP_AND: r[i.a] = r[i.b] & r[i.c]; break;
        case OP_OR: r[i.a] = r[i.b] | r[i.c]; break;
        case OP_XOR: r[i.a] = r[i.b] ^ r[i.c]; break;
        case OP_SHL: r[i.a] = r[i.b] << (r[i.c] & 31); break;
        case OP_SHR: r[i.a] = r[i.b] >> (r[i.c] & 31); break;
        case OP_SLT: r[i.a] = (int32_t)r[i.b] < (int32_t)r[i.c]; break;
        case OP_ADDI: r[i.a] = r[i.b] + (uint32_t)(int32_t)i.imm; break;
        case OP_LD: r[i.a] = m->mem[(r[i.b] + (uint32_t)(int32_t)i.imm) % MEMW]; break;
        case OP_ST: m->mem[(r[i.b] + (uint32_t)(int32_t)i.imm) % MEMW] = r[i.a]; break;
        case OP_BEQ: if (r[i.a] == r[i.b]) pc += (uint32_t)(int32_t)i.imm - 1; break;
        case OP_BNE: if (r[i.a] != r[i.b]) pc += (uint32_t)(int32_t)i.imm - 1; break;
        case OP_BLT: if ((int32_t)r[i.a] < (int32_t)r[i.b]) pc += (uint32_t)(int32_t)i.imm - 1; break;
        case OP_JMP: pc += (uint32_t)(int32_t)i.imm - 1; break;
        case OP_CALL: m->stack[m->sp++ & 255] = pc; pc += (uint32_t)(int32_t)i.imm - 1; break;
        case OP_RET: pc = m->stack[--m->sp & 255]; break;
        case OP_PUSH: m->stack[m->sp++ & 255] = r[i.a]; break;
        case OP_POP: r[i.a] = m->stack[--m->sp & 255]; break;
        default: m->halted = false; return;
        }
        r[0] = 0;
    }
}

/* ---- handlers ---- */

#define H(name) static uint32_t h_##name(vm *m, const insn *i, uint32_t pc)
#define R (m->r)
#define IMM ((uint32_t)(int32_t)i->imm)

H(halt) { m->halted = true; (void)i; (void)pc; return UINT32_MAX; }
H(li) { R[i->a] = IMM; return pc + 1; }
H(lui) { R[i->a] = (uint32_t)(uint16_t)i->imm << 16 | (R[i->a] & 0xffff); return pc + 1; }
H(mov) { R[i->a] = R[i->b]; return pc + 1; }
#define ARITH(name, expr) H(name) { uint32_t x = R[i->b], y = R[i->c]; R[i->a] = (expr); return pc + 1; }
ARITH(add, x + y)
ARITH(sub, x - y)
ARITH(mul, x * y)
ARITH(divu, y ? x / y : 0)
ARITH(remu, y ? x % y : x)
ARITH(and, x & y)
ARITH(or, x | y)
ARITH(xor, x ^ y)
ARITH(shl, x << (y & 31))
ARITH(shr, x >> (y & 31))
ARITH(slt, (int32_t)x < (int32_t)y)
#undef ARITH
H(addi) { R[i->a] = R[i->b] + IMM; return pc + 1; }
H(ld) { R[i->a] = m->mem[(R[i->b] + IMM) % MEMW]; return pc + 1; }
H(st) { m->mem[(R[i->b] + IMM) % MEMW] = R[i->a]; return pc + 1; }
H(beq) { return R[i->a] == R[i->b] ? pc + IMM : pc + 1; }
H(bne) { return R[i->a] != R[i->b] ? pc + IMM : pc + 1; }
H(blt) { return (int32_t)R[i->a] < (int32_t)R[i->b] ? pc + IMM : pc + 1; }
H(jmp) { (void)m; return pc + IMM; }
H(call) { m->stack[m->sp++ & 255] = pc + 1; return pc + IMM; }
H(ret) { (void)i; (void)pc; return m->stack[--m->sp & 255]; }
H(push) { m->stack[m->sp++ & 255] = R[i->a]; return pc + 1; }
H(pop) { R[i->a] = m->stack[--m->sp & 255]; return pc + 1; }
#undef H
#undef R
#undef IMM

static const handler table[NOPS] = {
    [OP_HALT] = h_halt, [OP_LI] = h_li, [OP_MOV] = h_mov, [OP_ADD] = h_add, [OP_SUB] = h_sub, [OP_MUL] = h_mul,
    [OP_DIVU] = h_divu, [OP_REMU] = h_remu, [OP_AND] = h_and, [OP_OR] = h_or, [OP_XOR] = h_xor,
    [OP_SHL] = h_shl, [OP_SHR] = h_shr, [OP_ADDI] = h_addi, [OP_LD] = h_ld, [OP_ST] = h_st, [OP_BEQ] = h_beq,
    [OP_BNE] = h_bne, [OP_BLT] = h_blt, [OP_JMP] = h_jmp, [OP_CALL] = h_call, [OP_RET] = h_ret,
    [OP_PUSH] = h_push, [OP_POP] = h_pop, [OP_SLT] = h_slt, [OP_LUI] = h_lui,
};

static void run_table(vm *m, const uint32_t *code, uint32_t n) {
    uint32_t pc = 0;
    while (pc < n && !m->halted) {
        insn i = decode(code[pc]);
        m->steps++;
        pc = table[i.op](m, &i, pc);
        m->r[0] = 0;
    }
}

static insn predecoded[1024];

static void run_threaded(vm *m, const uint32_t *code, uint32_t n) {
    for (uint32_t k = 0; k < n; k++) {
        predecoded[k] = decode(code[k]);
        predecoded[k].fn = table[predecoded[k].op];
    }
    const insn *base = predecoded;
    for (uint32_t pc = 0; pc < n && !m->halted;) {
        const insn *i = &base[pc];
        m->steps++;
        pc = i->fn(m, i, pc);
        m->r[0] = 0;
    }
}

/* ---- assembler ---- */

typedef struct {
    uint32_t code[1024];
    uint32_t n;
    int labels[32];
    struct { uint32_t at; int label; } fix[64];
    int nfix;
} asmb;

static void a_init(asmb *a) {
    a->n = 0;
    a->nfix = 0;
    for (int i = 0; i < 32; i++) a->labels[i] = -1;
}
static void put(asmb *a, uint32_t w) { a->code[a->n++] = w; }
static void label(asmb *a, int l) { a->labels[l] = (int)a->n; }
static void branch(asmb *a, enum opcode op, unsigned x, unsigned y, int l) {
    a->fix[a->nfix].at = a->n;
    a->fix[a->nfix++].label = l;
    put(a, enc_i(op, x, y, 0));
}
static int link(asmb *a) {
    int unresolved = 0;
    for (int k = 0; k < a->nfix; k++) {
        int target = a->labels[a->fix[k].label];
        if (target < 0) { unresolved++; continue; }
        uint32_t at = a->fix[k].at;
        a->code[at] = (a->code[at] & 0xffff0000u) | ((uint32_t)(target - (int)at) & 0xffff);
    }
    return unresolved;
}
static void li32(asmb *a, unsigned r, uint32_t v) {
    put(a, enc_i(OP_LI, r, 0, (int)(int16_t)(v & 0xffff)));
    put(a, enc_i(OP_LUI, r, 0, (int)(int16_t)(v >> 16)));
}

/* sieve of Eratosthenes over mem[0..limit), count in r1 */
static void prog_sieve(asmb *a, int limit) {
    enum { LOOP_I = 1, LOOP_J, NEXT_I, DONE, COUNT, CNEXT, CDONE };
    put(a, enc_i(OP_LI, 2, 0, 2));          /* r2 = i = 2 */
    put(a, enc_i(OP_LI, 3, 0, limit));      /* r3 = limit */
    put(a, enc_i(OP_LI, 4, 0, 1));          /* r4 = 1 (composite mark) */
    label(a, LOOP_I);
    put(a, enc(OP_MUL, 5, 2, 2));           /* r5 = i*i */
    branch(a, OP_BLT, 3, 5, DONE);          /* limit < i*i -> done */
    put(a, enc_i(OP_LD, 6, 2, 0));
    branch(a, OP_BNE, 6, 0, NEXT_I);        /* already composite */
    label(a, LOOP_J);
    branch(a, OP_BLT, 5, 3, 30);            /* j < limit -> mark (label 30) */
    branch(a, OP_JMP, 0, 0, NEXT_I);
    label(a, 30);
    put(a, enc_i(OP_ST, 4, 5, 0));
    put(a, enc(OP_ADD, 5, 5, 2));
    branch(a, OP_JMP, 0, 0, LOOP_J);
    label(a, NEXT_I);
    put(a, enc_i(OP_ADDI, 2, 2, 1));
    branch(a, OP_JMP, 0, 0, LOOP_I);
    label(a, DONE);
    put(a, enc_i(OP_LI, 1, 0, 0));
    put(a, enc_i(OP_LI, 2, 0, 2));
    label(a, COUNT);
    branch(a, OP_BLT, 2, 3, CNEXT);
    branch(a, OP_JMP, 0, 0, CDONE);
    label(a, CNEXT);
    put(a, enc_i(OP_LD, 6, 2, 0));
    put(a, enc(OP_XOR, 6, 6, 4));           /* 1 if prime */
    put(a, enc(OP_ADD, 1, 1, 6));
    put(a, enc_i(OP_ADDI, 2, 2, 1));
    branch(a, OP_JMP, 0, 0, COUNT);
    label(a, CDONE);
    put(a, enc(OP_HALT, 0, 0, 0));
}

/* recursive fib through call/ret and push/pop: r1 = fib(r2) */
static void prog_fib(asmb *a, int n) {
    enum { FIB = 1, BASE, END };
    put(a, enc_i(OP_LI, 2, 0, n));
    put(a, enc_i(OP_LI, 7, 0, 2));
    branch(a, OP_CALL, 0, 0, FIB);
    put(a, enc(OP_HALT, 0, 0, 0));
    label(a, FIB);
    branch(a, OP_BLT, 2, 7, BASE);
    put(a, enc_i(OP_PUSH, 2, 0, 0));
    put(a, enc_i(OP_ADDI, 2, 2, -1));
    branch(a, OP_CALL, 0, 0, FIB);
    put(a, enc_i(OP_POP, 2, 0, 0));
    put(a, enc_i(OP_PUSH, 1, 0, 0));
    put(a, enc_i(OP_ADDI, 2, 2, -2));
    branch(a, OP_CALL, 0, 0, FIB);
    put(a, enc_i(OP_ADDI, 2, 2, 2));
    put(a, enc_i(OP_POP, 8, 0, 0));
    put(a, enc(OP_ADD, 1, 1, 8));
    put(a, enc_i(OP_RET, 0, 0, 0));
    label(a, BASE);
    put(a, enc(OP_MOV, 1, 2, 0));
    put(a, enc_i(OP_RET, 0, 0, 0));
    (void)END;
}

/* fill mem with an LCG, bubble sort it, then hash it into r1 */
static void prog_sort(asmb *a, int n) {
    enum { FILL = 1, OUTER, INNER, NOSWAP, INNER_END, HASH, HASH_END, OUTER_END };
    li32(a, 9, 1664525u);
    li32(a, 10, 1013904223u);
    put(a, enc_i(OP_LI, 3, 0, n));
    put(a, enc_i(OP_LI, 2, 0, 0));
    put(a, enc_i(OP_LI, 11, 0, 12345));
    put(a, enc_i(OP_LI, 12, 0, 16));
    label(a, FILL);
    put(a, enc(OP_MUL, 11, 11, 9));
    put(a, enc(OP_ADD, 11, 11, 10));
    put(a, enc(OP_SHR, 6, 11, 12));
    put(a, enc_i(OP_ST, 6, 2, 100));
    put(a, enc_i(OP_ADDI, 2, 2, 1));
    branch(a, OP_BLT, 2, 3, FILL);
    put(a, enc_i(OP_ADDI, 4, 3, -1));          /* r4 = outer bound */
    label(a, OUTER);
    branch(a, OP_BEQ, 4, 0, OUTER_END);
    put(a, enc_i(OP_LI, 2, 0, 0));
    label(a, INNER);
    branch(a, OP_BLT, 2, 4, 20);
    branch(a, OP_JMP, 0, 0, INNER_END);
    label(a, 20);
    put(a, enc_i(OP_LD, 5, 2, 100));
    put(a, enc_i(OP_LD, 6, 2, 101));
    put(a, enc(OP_SLT, 13, 6, 5));
    branch(a, OP_BEQ, 13, 0, NOSWAP);
    put(a, enc_i(OP_ST, 6, 2, 100));
    put(a, enc_i(OP_ST, 5, 2, 101));
    label(a, NOSWAP);
    put(a, enc_i(OP_ADDI, 2, 2, 1));
    branch(a, OP_JMP, 0, 0, INNER);
    label(a, INNER_END);
    put(a, enc_i(OP_ADDI, 4, 4, -1));
    branch(a, OP_JMP, 0, 0, OUTER);
    label(a, OUTER_END);
    put(a, enc_i(OP_LI, 1, 0, 0));
    put(a, enc_i(OP_LI, 2, 0, 0));
    put(a, enc_i(OP_LI, 14, 0, 31));
    label(a, HASH);
    put(a, enc_i(OP_LD, 6, 2, 100));
    put(a, enc(OP_MUL, 1, 1, 14));
    put(a, enc(OP_ADD, 1, 1, 6));
    put(a, enc_i(OP_ADDI, 2, 2, 1));
    branch(a, OP_BLT, 2, 3, HASH);
    put(a, enc(OP_HALT, 0, 0, 0));
    (void)HASH_END;
}

/* arithmetic soup: every ALU operation in a counted loop */
static void prog_alu(asmb *a, int iters) {
    enum { LOOP = 1 };
    li32(a, 1, 0x12345678u);
    li32(a, 2, 0x9abcdef0u);
    put(a, enc_i(OP_LI, 3, 0, iters));
    put(a, enc_i(OP_LI, 4, 0, 0));
    put(a, enc_i(OP_LI, 15, 0, 7));
    label(a, LOOP);
    put(a, enc(OP_ADD, 5, 1, 2));
    put(a, enc(OP_SUB, 6, 1, 2));
    put(a, enc(OP_MUL, 7, 5, 6));
    put(a, enc(OP_DIVU, 8, 7, 15));
    put(a, enc(OP_REMU, 9, 7, 15));
    put(a, enc(OP_XOR, 1, 7, 8));
    put(a, enc(OP_SHL, 10, 9, 15));
    put(a, enc(OP_SHR, 11, 1, 15));
    put(a, enc(OP_OR, 2, 10, 11));
    put(a, enc(OP_AND, 12, 1, 2));
    put(a, enc(OP_SLT, 13, 1, 2));
    put(a, enc(OP_ADD, 2, 2, 13));
    put(a, enc(OP_ADD, 2, 2, 12));
    put(a, enc_i(OP_ADDI, 15, 15, 3));
    put(a, enc_i(OP_ADDI, 4, 4, 1));
    branch(a, OP_BLT, 4, 3, LOOP);
    put(a, enc(OP_XOR, 1, 1, 2));
    put(a, enc(OP_HALT, 0, 0, 0));
}

static vm machines[3];

static void run_all(const char *name, size_t len, void (*gen)(asmb *, int), int arg) {
    static asmb a;
    a_init(&a);
    gen(&a, arg);
    int unresolved = link(&a);
    void (*runners[3])(vm *, const uint32_t *, uint32_t) = { run_switch, run_table, run_threaded };
    uint32_t results[3];
    uint64_t steps[3];
    for (int k = 0; k < 3; k++) {
        vm *m = &machines[k];
        *m = (vm){ .sp = 0 };
        runners[k](m, a.code, a.n);
        results[k] = m->r[1];
        steps[k] = m->steps;
        if (!m->halted) results[k] = 0xdeadu;
    }
    uint32_t memh = 0;
    for (int k = 0; k < MEMW; k++) memh = memh * 33 + machines[0].mem[k];
    print_str(name, len);
    print_u64(results[0]);
    print_u64(steps[0]);
    print_bool(results[0] == results[1] && results[1] == results[2] && steps[0] == steps[1] && steps[1] == steps[2]);
    print_i64(unresolved * 1000 + (int)a.n);
    print_u64(memh);
}

int main(void) {
    run_all("sieve", 5, prog_sieve, 2000);
    run_all("fib", 3, prog_fib, 20);
    run_all("sort", 4, prog_sort, 150);
    run_all("alu", 3, prog_alu, 5000);
    insn x = decode(enc_i(OP_ADDI, 15, 9, -32768));
    print_i64(x.op * 1000 + x.a * 100 + x.b);
    print_i64(x.imm);
    x.imm = 32767;
    x.imm += 1 - 1;
    x.a = 17;   /* wraps to 1 */
    print_i64(x.imm + x.a);
    print_i64((int)sizeof(insn) >= 8);
    return 0;
}

/* expect: sieve */
/* expect: 303 */
/* expect: 24300 */
/* expect: true */
/* expect: 24 */
/* expect: 1659337503 */
/* expect: fib */
/* expect: 6765 */
/* expect: 164182 */
/* expect: true */
/* expect: 18 */
/* expect: 0 */
/* expect: sort */
/* expect: 3252156274 */
/* expect: 91723 */
/* expect: true */
/* expect: 38 */
/* expect: 224792738 */
/* expect: alu */
/* expect: 1527013374 */
/* expect: 80009 */
/* expect: true */
/* expect: 25 */
/* expect: 0 */
/* expect: 14509 */
/* expect: -32768 */
/* expect: 32768 */
/* expect: 1 */
