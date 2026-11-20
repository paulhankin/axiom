/* A register-based virtual machine of the kind a database engine runs queries with: a big
   switch in a loop over an array of operations, typed memory cells, a cursor over records
   decoded from varint-encoded rows, calls of functions through pointers, many values live
   across the loop. */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
enum { MEM_NULL, MEM_INT, MEM_REAL };
typedef struct { int flags; int n; union { int64_t i; double r; } u; } Mem;
typedef struct { uint8_t op; int p1, p2, p3; void *p4; } Op;
typedef struct { const uint8_t *data; size_t len, pos; int64_t rowid; int nfield; int64_t field[8]; } Cursor;
typedef void (*Func)(Mem *out, Mem *a, Mem *b);
enum { OP_Init, OP_Integer, OP_Real, OP_Copy, OP_Add, OP_Sub, OP_Mul, OP_Div, OP_Rem, OP_BitAnd,
       OP_ShiftR, OP_Lt, OP_Le, OP_Eq, OP_Ne, OP_Goto, OP_Rewind, OP_Next, OP_Column, OP_Rowid,
       OP_Function, OP_IfNot, OP_ResultRow, OP_AddImm, OP_Halt, OP_ToReal, OP_Max, OP_Null };
static int get_varint(const uint8_t *p, int64_t *v) {
    uint64_t x = 0; int i = 0;
    do { x = (x << 7) | (p[i] & 0x7f); } while (p[i++] & 0x80);
    *v = (int64_t)x; return i;
}
static int put_varint(uint8_t *p, uint64_t v) {
    uint8_t tmp[10]; int n = 0;
    do { tmp[n++] = (uint8_t)((v & 0x7f) | 0x80); v >>= 7; } while (v);
    tmp[0] &= 0x7f;
    for (int i = 0; i < n; i++) p[i] = tmp[n - 1 - i];
    return n;
}
static int cursor_next(Cursor *c) {
    if (c->pos >= c->len) return 0;
    c->pos += get_varint(c->data + c->pos, &c->rowid);
    c->nfield = c->data[c->pos++];
    for (int i = 0; i < c->nfield; i++) c->pos += get_varint(c->data + c->pos, &c->field[i]);
    return 1;
}
static void fn_hash(Mem *out, Mem *a, Mem *b) {
    uint64_t h = (uint64_t)a->u.i * 0x9E3779B97F4A7C15ull ^ (uint64_t)b->u.i;
    out->flags = MEM_INT; out->u.i = (int64_t)(h >> 7);
}
static void fn_min(Mem *out, Mem *a, Mem *b) {
    out->flags = MEM_INT; out->u.i = a->u.i < b->u.i ? a->u.i : b->u.i;
}
static double as_real(const Mem *m) { return m->flags == MEM_REAL ? m->u.r : (double)m->u.i; }
static int64_t exec(const Op *aOp, Mem *aMem, Cursor *cur, int64_t *out, int *nout) {
    int pc = 0, rows = 0;
    int64_t acc = 0, ops = 0;
    double racc = 0;
    for (;;) {
        const Op *o = &aOp[pc];
        ops++;
        switch (o->op) {
        case OP_Init: pc = o->p2; continue;
        case OP_Integer: aMem[o->p2].flags = MEM_INT; aMem[o->p2].u.i = o->p1; break;
        case OP_Real: aMem[o->p2].flags = MEM_REAL; aMem[o->p2].u.r = *(double *)o->p4; break;
        case OP_Null: aMem[o->p2].flags = MEM_NULL; break;
        case OP_Copy: aMem[o->p2] = aMem[o->p1]; break;
        case OP_Add: case OP_Sub: case OP_Mul: case OP_Div: case OP_Rem: case OP_BitAnd: case OP_ShiftR: {
            Mem *a = &aMem[o->p1], *b = &aMem[o->p2], *r = &aMem[o->p3];
            if (a->flags == MEM_INT && b->flags == MEM_INT) {
                int64_t x = a->u.i, y = b->u.i, z;
                switch (o->op) {
                case OP_Add: z = (int64_t)((uint64_t)x + (uint64_t)y); break;
                case OP_Sub: z = (int64_t)((uint64_t)x - (uint64_t)y); break;
                case OP_Mul: z = (int64_t)((uint64_t)x * (uint64_t)y); break;
                case OP_Div: z = y ? x / y : 0; break;
                case OP_Rem: z = y ? x % y : 0; break;
                case OP_BitAnd: z = x & y; break;
                default: z = (int64_t)((uint64_t)x >> (y & 63)); break;
                }
                r->flags = MEM_INT; r->u.i = z;
            } else if (a->flags == MEM_NULL || b->flags == MEM_NULL) {
                r->flags = MEM_NULL;
            } else {
                double x = as_real(a), y = as_real(b), z;
                switch (o->op) {
                case OP_Add: z = x + y; break;
                case OP_Sub: z = x - y; break;
                case OP_Mul: z = x * y; break;
                case OP_Div: z = y != 0 ? x / y : 0; break;
                default: z = x; break;
                }
                r->flags = MEM_REAL; r->u.r = z;
            }
            break;
        }
        case OP_Lt: case OP_Le: case OP_Eq: case OP_Ne: {
            Mem *a = &aMem[o->p1], *b = &aMem[o->p3];
            int c;
            if (a->flags == MEM_INT && b->flags == MEM_INT) c = a->u.i < b->u.i ? -1 : a->u.i > b->u.i;
            else { double x = as_real(a), y = as_real(b); c = x < y ? -1 : x > y; }
            int t = o->op == OP_Lt ? c < 0 : o->op == OP_Le ? c <= 0 : o->op == OP_Eq ? c == 0 : c != 0;
            if (t) { pc = o->p2; continue; }
            break;
        }
        case OP_Goto: pc = o->p2; continue;
        case OP_Rewind: cur->pos = 0; if (!cursor_next(cur)) { pc = o->p2; continue; } break;
        case OP_Next: if (cursor_next(cur)) { pc = o->p2; continue; } break;
        case OP_Column: {
            Mem *r = &aMem[o->p3];
            if (o->p2 < cur->nfield) { r->flags = MEM_INT; r->u.i = cur->field[o->p2]; } else r->flags = MEM_NULL;
            break;
        }
        case OP_Rowid: aMem[o->p2].flags = MEM_INT; aMem[o->p2].u.i = cur->rowid; break;
        case OP_Function: ((Func)o->p4)(&aMem[o->p3], &aMem[o->p1], &aMem[o->p2]); break;
        case OP_IfNot: if (aMem[o->p1].flags == MEM_NULL || aMem[o->p1].u.i == 0) { pc = o->p2; continue; } break;
        case OP_AddImm: aMem[o->p1].u.i += o->p2; break;
        case OP_ToReal: if (aMem[o->p1].flags == MEM_INT) { aMem[o->p1].u.r = (double)aMem[o->p1].u.i; aMem[o->p1].flags = MEM_REAL; } break;
        case OP_Max: if (aMem[o->p1].u.i > aMem[o->p2].u.i) aMem[o->p2].u.i = aMem[o->p1].u.i; break;
        case OP_ResultRow:
            rows++;
            for (int i = 0; i < o->p2; i++) {
                Mem *m = &aMem[o->p1 + i];
                if (m->flags == MEM_INT) acc = acc * 31 + m->u.i;
                else if (m->flags == MEM_REAL) racc += m->u.r;
            }
            if (*nout < 16) out[(*nout)++] = acc;
            break;
        case OP_Halt: return acc + rows + ops + (int64_t)racc;
        }
        pc++;
    }
}
int main(void) {
    size_t cap = 1 << 22;
    uint8_t *data = malloc(cap);
    size_t p = 0; uint64_t s = 12345; int64_t id = 0;
    while (p + 80 < cap) {
        s = s * 6364136223846793005ull + 1442695040888963407ull;
        int nf = 3 + (int)((s >> 61) & 3);
        p += put_varint(data + p, (uint64_t)id++);
        data[p++] = (uint8_t)nf;
        for (int f = 0; f < nf; f++) p += put_varint(data + p, ((s >> 20) >> (f * 7)) & ((1ull << (f * 6 + 4)) - 1));
    }
    static double half = 0.5;
    /* for each row: r1 = col0, r2 = col1, r3 = col2; if r1 < 9 skip; r4 = r1*r2 + r3; r5 = hash(r4, rowid);
       r6 = min(r5, r2); max r7; r8 = col3 as real * 0.5 (NULL when absent); every 64th row: result row */
    Op prog[] = {
        {OP_Init, 0, 1, 0, 0},
        {OP_Integer, 0, 7, 0, 0}, {OP_Integer, 15, 10, 0, 0}, {OP_Real, 0, 11, 0, &half},
        {OP_Rewind, 0, 24, 0, 0},
        {OP_Column, 0, 0, 1, 0}, {OP_Column, 0, 1, 2, 0}, {OP_Column, 0, 2, 3, 0},
        {OP_Integer, 9, 12, 0, 0}, {OP_Lt, 1, 23, 12, 0},
        {OP_Mul, 1, 2, 4, 0}, {OP_Add, 4, 3, 4, 0}, {OP_Rowid, 0, 9, 0, 0},
        {OP_Function, 4, 9, 5, (void *)fn_hash}, {OP_Function, 5, 2, 6, (void *)fn_min},
        {OP_Max, 6, 7, 0, 0}, {OP_Column, 0, 3, 8, 0}, {OP_Mul, 8, 11, 8, 0},
        {OP_BitAnd, 9, 10, 13, 0}, {OP_IfNot, 13, 21, 0, 0}, {OP_Goto, 0, 23, 0, 0},
        {OP_Copy, 6, 14, 0, 0}, {OP_ResultRow, 4, 5, 0, 0},
        {OP_Next, 0, 5, 0, 0},
        {OP_Halt, 0, 0, 0, 0},
    };
    Mem mem[16]; memset(mem, 0, sizeof mem);
    Cursor c = {data, p, 0, 0, 0, {0}};
    int64_t out[16]; int nout = 0; int64_t total = 0;
    for (int rep = 0; rep < 40; rep++) { nout = 0; total += exec(prog, mem, &c, out, &nout); }
    printf("vm: %lld %d %lld %lld\n", (long long)total, nout, (long long)out[nout - 1], (long long)mem[7].u.i);
    return 0;
}
