/* flags: --c-std=c99 --c-int=16 */
/* A program written for a 16-bit int: an emulator of a small 16-bit
   accumulator machine whose registers are unsigned int (wrapping at
   65536) with carry and overflow flags computed the portable way, a
   Fletcher-16 checksum, a 16-bit Galois LFSR, Q8.8 fixed-point
   arithmetic, binary-coded decimal addition, and an insertion sort of
   ints. The first section checks what a 16-bit int implies: the limits,
   the promotions of unsigned short (to unsigned int) and of unsigned char
   (to int), the types of constants too large for int, and wrapping
   unsigned arithmetic. */
#include "host.h"
#include <limits.h>
#include <stdint.h>

enum op { LDA, LDI, STA, ADD, ADC, SUB, AND, OR, XOR, SHL, SHR, JMP, JZ, JC, JN, INC, DEC, OUT, HLT };

struct cpu {
    unsigned acc, pc, sp;
    unsigned mem[256];
    int carry, zero, neg, ovf;
    unsigned out_hash;
    int outputs;
    long cycles;
};

static void flags(struct cpu *c, unsigned r) {
    c->zero = r == 0;
    c->neg = (r & 0x8000u) != 0;
}

static void add(struct cpu *c, unsigned b, int with_carry) {
    unsigned a = c->acc;
    unsigned cin = with_carry ? (unsigned)c->carry : 0u;
    unsigned s = a + b;
    int carry = s < a;
    unsigned t = s + cin;
    carry |= t < s;
    /* signed overflow: operands agree in sign and the result does not */
    c->ovf = (~(a ^ b) & (a ^ t) & 0x8000u) != 0;
    c->carry = carry;
    c->acc = t;
    flags(c, t);
}

static void run(struct cpu *c, long max) {
    while (c->cycles < max) {
        unsigned w = c->mem[c->pc & 0xffu];
        unsigned op = w >> 8, arg = w & 0xffu;
        c->pc = (c->pc + 1) & 0xffu;
        c->cycles++;
        switch (op) {
        case LDA: c->acc = c->mem[arg]; flags(c, c->acc); break;
        case LDI: c->acc = arg; flags(c, c->acc); break;
        case STA: c->mem[arg] = c->acc; break;
        case ADD: add(c, c->mem[arg], 0); break;
        case ADC: add(c, c->mem[arg], 1); break;
        case SUB: add(c, ~c->mem[arg], 0); add(c, 1u, 0); break;
        case AND: c->acc &= c->mem[arg]; flags(c, c->acc); break;
        case OR: c->acc |= c->mem[arg]; flags(c, c->acc); break;
        case XOR: c->acc ^= c->mem[arg]; flags(c, c->acc); break;
        case SHL: c->carry = (c->acc & 0x8000u) != 0; c->acc <<= 1; flags(c, c->acc); break;
        case SHR: c->carry = c->acc & 1u; c->acc >>= 1; flags(c, c->acc); break;
        case JMP: c->pc = arg; break;
        case JZ: if (c->zero) c->pc = arg; break;
        case JC: if (c->carry) c->pc = arg; break;
        case JN: if (c->neg) c->pc = arg; break;
        case INC: c->acc++; flags(c, c->acc); break;
        case DEC: c->acc--; flags(c, c->acc); break;
        case OUT:
            c->out_hash = c->out_hash * 31u + c->acc;
            c->outputs++;
            break;
        case HLT: return;
        default: return;
        }
    }
}

#define I(op, arg) ((unsigned)(op) << 8 | (unsigned)(arg))

/* fib numbers until the 16-bit sum carries out; then a multiply by shifts */
static const unsigned program[] = {
    I(LDI, 0), I(STA, 200), I(LDI, 1), I(STA, 201),     /* a = 0, b = 1 */
    /* 4: */ I(LDA, 200), I(ADD, 201), I(JC, 13), I(OUT, 0),
    I(STA, 202), I(LDA, 201), I(STA, 200), I(LDA, 202),
    /* 13: the carry out of the sum halts */
    I(JMP, 16), I(HLT, 0), I(HLT, 0), I(HLT, 0),
    /* 16: */ I(STA, 201), I(JMP, 4),
};

static unsigned fletcher16(const unsigned char *p, int n) {
    unsigned s1 = 0, s2 = 0;
    int i;
    for (i = 0; i < n; i++) {
        s1 = (s1 + p[i]) % 255u;
        s2 = (s2 + s1) % 255u;
    }
    return s2 << 8 | s1;
}

static unsigned lfsr_step(unsigned x) {
    unsigned lsb = x & 1u;
    x >>= 1;
    if (lsb) x ^= 0xB400u;
    return x;
}

typedef int q8;   /* Q8.8 in a 16-bit int: values within +-127 */
static q8 qmul(q8 a, q8 b) { return (q8)(((long)a * b) >> 8); }
static q8 qdiv(q8 a, q8 b) { return (q8)(((long)a << 8) / b); }

/* packed BCD: four digits per unsigned */
static unsigned bcd_add(unsigned a, unsigned b, int *carry_out) {
    unsigned r = 0;
    int carry = 0, shift;
    for (shift = 0; shift < 16; shift += 4) {
        int d = (int)((a >> shift) & 15u) + (int)((b >> shift) & 15u) + carry;
        carry = d > 9;
        if (carry) d -= 10;
        r |= (unsigned)d << shift;
    }
    *carry_out = carry;
    return r;
}

static void isort(int *a, int n) {
    int i, j;
    for (i = 1; i < n; i++) {
        int v = a[i];
        for (j = i; j > 0 && a[j - 1] > v; j--) a[j] = a[j - 1];
        a[j] = v;
    }
}

int main(void) {
    /* what a 16-bit int means */
    {
        unsigned short us = 65535;
        unsigned char uc = 200;
        unsigned u = 65535u;
        int i = INT_MAX;
        print_i64(sizeof(int));
        print_i64(INT_MAX);
        print_i64(INT_MIN);
        print_u64(UINT_MAX);
        print_i64(sizeof(us + 0) == sizeof(unsigned));   /* unsigned short promotes to unsigned int */
        print_u64(us + 1u);                               /* wraps to 0 */
        print_i64(us + 1 == 0);                           /* still unsigned arithmetic */
        print_i64(uc * 100 / 100);                        /* 20000 fits in int */
        print_i64(sizeof 40000 == sizeof(long));          /* too big for int: long */
        print_i64(sizeof 0xFFFF == sizeof(unsigned));     /* hexadecimal fits unsigned int */
        print_i64(sizeof 32767 == sizeof(int));
        print_i64(-1 < 0u);                               /* -1 converts to 65535u: false */
        print_i64(-1L < 0u);                              /* long holds every unsigned value */
        u++;
        print_u64(u);
        print_i64(i - 1);
        print_i64((int)sizeof(long) >= 4);
        print_u64((unsigned)-1 >> 15);
        print_i64(1 << 14);
        print_u64(1u << 15);
        print_i64('A' * 256 + 'B');                       /* 16706 fits */
        print_i64((long)INT_MAX + 1);
    }

    /* the emulator */
    {
        static struct cpu c;
        unsigned k;
        for (k = 0; k < sizeof program / sizeof program[0]; k++) c.mem[k] = program[k];
        run(&c, 100000L);
        print_i64(c.outputs);
        print_u64(c.out_hash);
        print_u64(c.mem[201]);
        print_u64(c.mem[200]);
        print_i64(c.carry);
        print_i64(c.cycles);
        /* flags of a few additions */
        c.acc = 0x7fffu; add(&c, 1u, 0);
        print_i64(c.ovf * 1000 + c.carry * 100 + c.neg * 10 + c.zero);
        c.acc = 0xffffu; add(&c, 1u, 0);
        print_i64(c.ovf * 1000 + c.carry * 100 + c.neg * 10 + c.zero);
        c.acc = 0x8000u; add(&c, 0x8000u, 0);
        print_i64(c.ovf * 1000 + c.carry * 100 + c.neg * 10 + c.zero);
        c.carry = 1; c.acc = 0xfffeu; add(&c, 1u, 1);
        print_i64(c.ovf * 1000 + c.carry * 100 + c.neg * 10 + c.zero);
    }

    /* checksums and sequences */
    {
        static unsigned char buf[300];
        unsigned x = 0xACE1u, period = 0, i;
        int j;
        for (j = 0; j < 300; j++) buf[j] = (unsigned char)(j * 7 + 3);
        print_u64(fletcher16(buf, 300));
        print_u64(fletcher16((const unsigned char *)"abcde", 5));
        do {
            x = lfsr_step(x);
            period++;
        } while (x != 0xACE1u && period != 0);
        print_u64(period);
        x = 1u;
        for (i = 0; i < 1000u; i++) x = lfsr_step(x);
        print_u64(x);
    }

    /* fixed point */
    {
        q8 a = 3 * 256 + 128;   /* 3.5 */
        q8 b = -2 * 256 - 64;   /* -2.25 */
        q8 s = 0;
        int k;
        print_i64(qmul(a, b));
        print_i64(qdiv(a, b));
        for (k = 1; k <= 40; k++) s += qdiv(256, k * 256);   /* harmonic sum in Q8.8 */
        print_i64(s);
    }

    /* BCD */
    {
        int carry;
        unsigned r = bcd_add(0x1999u, 0x0001u, &carry);
        print_u64(r);
        r = bcd_add(0x9999u, 0x0002u, &carry);
        print_u64(r * 10u + (unsigned)carry);
        r = 0;
        for (carry = 0; r < 0x5000u;) {
            int c2;
            r = bcd_add(r, 0x0789u, &c2);
            carry += c2;
            if (c2) break;
        }
        print_u64(r);
    }

    /* sorting ints */
    {
        int a[40], k;
        unsigned seed = 7u;
        long sum = 0;
        for (k = 0; k < 40; k++) {
            seed = seed * 25173u + 13849u;   /* wraps at 65536 */
            a[k] = (int)(seed >> 1) - 16384;
        }
        isort(a, 40);
        for (k = 0; k < 40; k++) sum += (long)a[k] * (k + 1);
        print_i64(a[0]);
        print_i64(a[39]);
        print_i64(sum);
    }
    return 0;
}

/* expect: 2 */
/* expect: 32767 */
/* expect: -32768 */
/* expect: 65535 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 200 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 32766 */
/* expect: 1 */
/* expect: 1 */
/* expect: 16384 */
/* expect: 32768 */
/* expect: 16706 */
/* expect: 32768 */
/* expect: 23 */
/* expect: 60433 */
/* expect: 46368 */
/* expect: 28657 */
/* expect: 1 */
/* expect: 261 */
/* expect: 1010 */
/* expect: 101 */
/* expect: 1101 */
/* expect: 101 */
/* expect: 35701 */
/* expect: 51440 */
/* expect: 65535 */
/* expect: 40959 */
/* expect: -2016 */
/* expect: -398 */
/* expect: 1080 */
/* expect: 8192 */
/* expect: 11 */
/* expect: 21795 */
/* expect: -15442 */
/* expect: 14933 */
/* expect: 2426009 */
