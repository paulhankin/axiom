/* Windows x64: the system's unwinder through every kind of frame the
   compiler makes, at every instruction. The functions below (a leaf, one
   that keeps integers and one that keeps doubles in callee-saved registers,
   one with variable length arrays (its stack pointer moves), a chain of
   sibling calls, direct and through a pointer, with stack arguments, one
   with a frame over 64 KiB (the prologue's stack probe), one with inline
   assembly that clobbers callee-saved registers, a caller of setjmp and a
   longjmp out of a callee, and an Axiom function) run twice:

   - single-stepped (the trap flag; DebugBreak starts it): at each
     instruction inside them, the walk RtlLookupFunctionEntry and
     RtlVirtualUnwind make from that instruction's context (a function
     without an entry is a leaf: its return address at rsp) reaches the
     caller's return address with the caller's stack pointer and its
     callee-saved registers (rbx, rbp, rsi, rdi, r12-r15, xmm6-xmm15) as
     they were when it called;
   - with an exception raised at each point of interest, which no handler
     takes: the system's dispatcher walks every frame to the thread's start,
     whose handler calls the unhandled-exception filter (the test's, which
     continues), so each raise reaching the filter is a walk that went
     through all of them. */
/* windows-only */
/* x86-64-only */
/* flags: --c-std=gnu11 */
/* uses: ../print.ax */
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include "../print.h"

/* kernel32.dll and ntdll.dll (by their names' rules) */
void *AddVectoredExceptionHandler(unsigned long first, long (*handler)(void *));
void *SetUnhandledExceptionFilter(long (*filter)(void *));
void DebugBreak(void);
void RaiseException(unsigned long code, unsigned long flags, unsigned long n, const unsigned long long *args);
void *GetModuleHandleW(const unsigned short *name);
void *RtlLookupFunctionEntry(unsigned long long pc, unsigned long long *base, void *history);
void *RtlVirtualUnwind(unsigned long type, unsigned long long base, unsigned long long pc, void *entry, void *context,
                       void **data, unsigned long long *frame, void *pointers);

/* the x64 CONTEXT: its size, and its fields' offsets */
#define CTX_SIZE 0x4d0
#define EFLAGS 0x44
#define RSP 0x98
#define RIP 0xf8
#define XMM6 0x200
#define REG(c, off) (*(unsigned long long *)((unsigned char *)(c) + (off)))
static const int saved_regs[] = {0x90, 0xa0, 0xa8, 0xb0, 0xd8, 0xe0, 0xe8, 0xf0}; /* rbx rbp rsi rdi r12-r15 */

long long ax_frames(long long n); /* walk.ax */

static unsigned long long text_lo, text_hi;
static unsigned long long entry_pc, stop_pc, ret_pc, root_rsp;
static unsigned long long root_regs[8];
static unsigned char root_xmm[160];
static int stepping, active;
static long long steps, checked, failures, first_bad = -1, bad_kind;
static _Alignas(16) unsigned char walk[CTX_SIZE];

/* the walk from context c to the caller of the stepped function */
static int walk_ok(const unsigned char *c) {
    memcpy(walk, c, CTX_SIZE);
    for (int n = 0; n < 4000; n++) {
        unsigned long long pc = REG(walk, RIP), base = 0, frame = 0;
        void *data = 0;
        if (pc == ret_pc) {
            if (REG(walk, RSP) != root_rsp) return 1;
            for (int k = 0; k < 8; k++)
                if (REG(walk, saved_regs[k]) != root_regs[k]) return 2 + k;
            return memcmp(walk + XMM6, root_xmm, 160) == 0 ? 0 : 10;
        }
        if (pc < text_lo || pc >= text_hi) return 11;
        void *f = RtlLookupFunctionEntry(pc, &base, 0);
        if (f) {
            RtlVirtualUnwind(0, base, pc, f, walk, &data, &frame, 0);
        } else {
            REG(walk, RIP) = *(unsigned long long *)REG(walk, RSP);
            REG(walk, RSP) += 8;
        }
    }
    return 12;
}

static long on_exception(void *ep) {
    void **p = ep;
    unsigned code = *(unsigned *)p[0];
    unsigned char *c = p[1];
    unsigned long long pc = REG(c, RIP);
    if (code == 0x80000003u && !stepping) {
        /* DebugBreak's int3 (the context's pc is at it, or after it) */
        if (*(unsigned char *)pc == 0xcc) REG(c, RIP) = pc + 1;
        stepping = 1;
        REG(c, EFLAGS) |= 0x100;
        return -1; /* EXCEPTION_CONTINUE_EXECUTION */
    }
    if (code != 0x80000004u || !stepping) return 0; /* EXCEPTION_CONTINUE_SEARCH */
    if (pc == stop_pc) {
        stepping = 0;
        REG(c, EFLAGS) &= ~0x100ull;
        return -1;
    }
    if (pc == entry_pc && !active) {
        active = 1;
        ret_pc = *(unsigned long long *)REG(c, RSP);
        root_rsp = REG(c, RSP) + 8;
        for (int k = 0; k < 8; k++) root_regs[k] = REG(c, saved_regs[k]);
        memcpy(root_xmm, c + XMM6, 160);
    } else if (active && pc == ret_pc && REG(c, RSP) == root_rsp) {
        active = 0;
    }
    if (active) {
        steps++;
        if (pc >= text_lo && pc < text_hi) {
            int r = walk_ok(c);
            checked++;
            if (r) {
                if (first_bad < 0) {
                    first_bad = (long long)(pc - text_lo);
                    bad_kind = r;
                }
                failures++;
            }
        }
    }
    REG(c, EFLAGS) |= 0x100;
    return -1;
}

static long filtered;

static long on_unhandled(void *ep) {
    void **p = ep;
    if (*(unsigned *)p[0] != 0xe0000042u) return 0; /* EXCEPTION_CONTINUE_SEARCH */
    filtered++;
    return -1; /* EXCEPTION_CONTINUE_EXECUTION: RaiseException returns */
}

/* The points of interest: each counts, and raises the exception when asked. */
static volatile int raising;
static long long hits;

static void point(void) {
    hits++;
    if (raising) RaiseException(0xe0000042u, 0, 0, 0);
}
static void (*volatile point_p)(void) = point;

void point_c(void) { point_p(); }

static long long leaf(long long x) { return x * 3 + 1; }
static long long (*volatile leaf_p)(long long) = leaf;

static long long ints(long long x) {
    long long a = x + 1, b = x * 2, c = x ^ 5, d = x - 7, e = x * x, f = x + 11, g = x * 3, h = x | 64;
    point_p();
    long long r = leaf_p(a) + b * c + d * e + f * g + h;
    point_p();
    return r + a + b + c + d + e + f + g + h;
}

static double floats(double x) {
    double a = x + 1, b = x * 2, c = x / 3, d = x - 7, e = x * x, f = x + 11, g = x * 3, h = x / 5, i = x + 0.5,
           j = x * 0.25, k = x - 0.125;
    point_p();
    double r = a * b + c * d + e * f + g * h + i * j + k;
    point_p();
    return r + a + b + c + d + e + f + g + h + i + j + k;
}

static long long vla(int n) {
    volatile long long a[n];
    for (int i = 0; i < n; i++) a[i] = i;
    point_p();
    volatile long long b[n * 2 + 1];
    b[0] = a[n - 1];
    point_p();
    return a[n - 1] + b[0];
}

static long long sib_b(long long n, long long a, long long b, long long c, long long d, long long e);
static long long sib_a(long long n, long long a, long long b, long long c, long long d, long long e) {
    if (n == 0) {
        point_p();
        return a + b + c + d + e;
    }
    return sib_b(n - 1, b, c, d, e, a + 1);
}
static long long (*volatile sib_ap)(long long, long long, long long, long long, long long, long long) = sib_a;
static long long sib_b(long long n, long long a, long long b, long long c, long long d, long long e) {
    if (n % 3 == 0) return sib_ap(n, e, d, c, b, a); /* through a pointer */
    return sib_a(n, a, b, c, d, e + 2);
}

static long long big(int k) {
    volatile char buf[100000];
    buf[k] = 1;
    buf[99999 - k] = 2;
    point_p();
    return buf[k] + buf[99999 - k];
}

static long long with_asm(long long x) {
    long long y = x;
    __asm__ volatile("addq $5, %0\n\txorl %%ebx, %%ebx\n\tpxor %%xmm6, %%xmm6" : "+r"(y) : : "rbx", "r12", "xmm6", "xmm7");
    point_p();
    return y;
}

static jmp_buf env;
static void jumper(int v) {
    point_p();
    longjmp(env, v);
}
static void (*volatile jumper_p)(int) = jumper;
static long long sj(long long x) {
    volatile long long r = x;
    int v = setjmp(env);
    if (v == 0) {
        jumper_p(7);
        return -1;
    }
    point_p();
    return r + v;
}

static long long all(long long x) {
    long long s = leaf_p(x);
    s += ints(x);
    s += (long long)floats((double)x);
    s += vla(5 + (int)(x & 3));
    s += sib_a(20, x, 2, 3, 4, 5);
    s += big((int)x);
    s += with_asm(x);
    s += sj(x);
    s += ax_frames(x);
    return s;
}
static long long (*volatile all_p)(long long) = all;

static void stop(void) {}
static void (*volatile stop_p)(void) = stop;

int main(void) {
    unsigned char *image = GetModuleHandleW(0);
    unsigned pe = *(unsigned *)(image + 0x3c);
    unsigned char *text = image + pe + 24 + 240; /* the first section header: .text */
    text_lo = (unsigned long long)image + *(unsigned *)(text + 12);
    text_hi = text_lo + *(unsigned *)(text + 8);
    AddVectoredExceptionHandler(1, on_exception);
    SetUnhandledExceptionFilter(on_unhandled);
    long long want = all_p(3);
    long long n = hits;
    print_bool(n == 12); /* expect: true */
    /* every raise is walked to the thread's start, and continued */
    raising = 1;
    long long got = all_p(3);
    raising = 0;
    print_bool(got == want);      /* expect: true */
    print_bool(filtered == n);    /* expect: true */
    /* every instruction's walk */
    entry_pc = (unsigned long long)all;
    stop_pc = (unsigned long long)stop;
    DebugBreak();
    got = all_p(3);
    stop_p();
    print_bool(got == want);              /* expect: true */
    print_bool(steps > 1000 && checked > 1000); /* expect: true */
    print_i64(failures);                  /* expect: 0 */
    if (failures || getenv("WALK_COUNTS")) {
        print_i64(steps);
        print_i64(checked);
        print_i64(first_bad);
        print_i64(bad_kind);
    }
    return 0;
}
