/* flags: --c-std=gnu17 */
/* native-only */
/* AArch64's machine constraints and operand modifiers, run (under the
   user-mode emulator): memory as Q, Ump and the SVE forms (Utf), symbols (S,
   Usa, Ush), the constants Z, Y (the floating zero), Dz and Dn, the modifiers
   %w %x %b %h %s %d %q %z %c %n, and the registers the compiler does not
   allocate, through memory: SVE predicates (Upa, Upl), SVE vectors of objects
   wider than 16 bytes (w, x, y), and objects of 16 bytes or fewer in a q
   register. Elsewhere the same values are printed. */
#include "host.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef long long i64;
typedef unsigned long long u64;

static i64 gv = 1234;
typedef struct { u32 w[8]; } v8;   /* 32 bytes: an SVE z register's first lanes */
typedef struct { u32 w[4]; } v4;   /* 16 bytes: a q register */
typedef struct { u8 b[32]; } pred; /* an SVE predicate register's largest size */

#ifdef __AXIOM_ASM_AARCH64__
static i64 sym_load(void) {
    i64 r;
    /* Ush: the symbol's page for adrp; S its offset in it */
    asm("adrp %0, %1\n\tadd %0, %0, :lo12:%2\n\tldr %0, [%0]" : "=r"(r) : "Ush"(&gv), "S"(&gv));
    return r;
}
static i64 sym_abs(void) {
    i64 r;
    asm("adr %0, %1\n\tldr %0, [%0]" : "=r"(r) : "Usa"(&gv));
    return r;
}
static i64 pair(i64 *p) {
    i64 a, b;
    asm("ldp %0, %1, %2" : "=r"(a), "=r"(b) : "Ump"(*(struct { i64 x[2]; } *)p));
    return a * 100 + b;
}
static int load_q(int *p) {
    int r;
    asm("ldr %w0, %1" : "=r"(r) : "Q"(*p));
    return r;
}
static int fzero(double d) {
    int lt;
    asm("fcmp %d1, %2\n\tcset %w0, mi" : "=r"(lt) : "w"(d), "Y"(0.0) : "cc");
    return lt;
}
#endif

int main(void) {
#ifdef __AXIOM_ASM_AARCH64__
    print_i64(sym_load());          /* 1234 */
    print_i64(sym_abs());           /* 1234 */
    i64 two[2] = {5, 7};
    print_i64(pair(two));           /* 507 */
    int q = 99;
    print_i64(load_q(&q));          /* 99 */
    print_i64(fzero(-2.0));         /* 1 */
    print_i64(fzero(3.0));          /* 0 */
    /* Z and Dz: the zero register and #0 */
    i64 z;
    asm("mov %x0, %x1\n\tcmeq v31.4s, v31.4s, %2" : "=r"(z) : "Z"(0), "Dz"(0) : "v31");
    print_i64(z);                   /* 0 */
    /* Dn: a vector immediate; %c a bare constant, %n its negation */
    v4 m;
    asm("movi %0.4s, %1" : "=w"(m) : "Dn"(12));
    print_i64(m.w[0] + m.w[3]);     /* 24 */
    i64 cn;
    asm("mov %0, #%c1\n\tadd %0, %0, #%n2" : "=r"(cn) : "i"(40), "i"(-2));
    print_i64(cn);                  /* 42 */
    /* the views of a register: %w %x of a general one, %b %h %s %d %q %z of a vector one */
    i64 wide = -1;
    u32 low;
    asm("mov %w0, %w1" : "=r"(low) : "r"(wide));
    print_i64(low);                 /* 4294967295 */
    double d = 1.5, e;
    asm("fmov %d0, %d1\n\tmov %z0.d, %z0.d\n\tfadd %d0, %d0, %d0" : "=&w"(e) : "w"(d));
    print_i64((i64)(e * 10));       /* 30 */
    float f = 2.0f, g;
    asm("fmov %s0, %s1\n\tfadd %s0, %s0, %s0" : "=w"(g) : "w"(f));
    print_i64((i64)g);              /* 4 */
    u8 by = 0;
    asm("movi v30.16b, #7\n\tmov %b0, v30.b[0]\n\tmov %h0, v30.h[0]" : "=w"(d) : : "v30");
    u64 bits;
    __builtin_memcpy(&bits, &d, 8);
    print_i64((i64)(bits & 0xffff)); /* 1799 */
    (void)by;
    v4 src = {{3, 5, 0, 0}};
    asm("ldr %q0, %1" : "=w"(d) : "m"(src));
    __builtin_memcpy(&bits, &d, 8);
    print_i64((i64)(bits >> 32) * 10 + (i64)(bits & 0xffffffff)); /* 53 */
    /* an object of 16 bytes in a vector register (through a q register) */
    v4 a4 = {{1, 2, 3, 4}}, r4;
    asm("add %0.4s, %1.4s, %1.4s" : "=w"(r4) : "w"(a4));
    print_i64(r4.w[0] + 10 * r4.w[3]); /* 82 */
    /* SVE: a predicate (Upa) of the first four word lanes, used as a governing
       predicate (Upl) of a predicated add of 32-byte objects in z registers
       (x: z0-z15, y: z0-z7), and read back through memory */
    pred pg;
    asm("mov x9, #4\n\twhilelo %0.s, xzr, x9" : "=Upa"(pg) : : "x9");
    print_i64(pg.b[0] == 0x11 && pg.b[1] == 0x11 && pg.b[2] == 0); /* 1 */
    v8 a = {{1, 2, 3, 4, 5, 6, 7, 8}}, b = {{10, 20, 30, 40, 50, 60, 70, 80}}, c;
    asm("movprfx %0, %1\n\tadd %0.s, %3/m, %0.s, %2.s" : "=&w"(c) : "x"(a), "y"(b), "Upl"(pg));
    print_i64(c.w[0]);              /* 11 */
    print_i64(c.w[3]);              /* 44 */
    print_i64(c.w[4]);              /* 5 */
    print_i64(c.w[7]);              /* 8 */
    /* a read-write SVE operand, and Utf: memory for a whole-register load */
    asm("add %0.s, %0.s, %0.s" : "+w"(c));
    print_i64(c.w[0] + c.w[7]);     /* 38 */
    v8 ld;
    asm("ptrue p6.s\n\tld1w {z29.s}, p6/z, %1\n\tmov %0.d, z29.d"
        : "=w"(ld) : "Utf"(a) : "p6", "z29");
    print_i64(ld.w[0] + ld.w[7]);   /* 9 */
#else
    static const i64 v[] = {1234, 1234, 507, 99, 1, 0, 0, 24, 42, 4294967295LL, 30, 4, 1799, 53, 82, 1, 11, 44, 5, 8, 38, 9};
    for (unsigned i = 0; i < sizeof v / sizeof v[0]; i++) print_i64(v[i]);
#endif
    return 0;
}
/* expect: 1234 */
/* expect: 1234 */
/* expect: 507 */
/* expect: 99 */
/* expect: 1 */
/* expect: 0 */
/* expect: 0 */
/* expect: 24 */
/* expect: 42 */
/* expect: 4294967295 */
/* expect: 30 */
/* expect: 4 */
/* expect: 1799 */
/* expect: 53 */
/* expect: 82 */
/* expect: 1 */
/* expect: 11 */
/* expect: 44 */
/* expect: 5 */
/* expect: 8 */
/* expect: 38 */
/* expect: 9 */
