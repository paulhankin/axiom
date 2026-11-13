/* flags: --c-std=gnu17 */
/* wasm-only */
/* Inline assembly on WebAssembly (docs/c.md "Inline assembly"): templates
   in the text format, plain and folded, with operands in locals (inputs,
   outputs, '+', a matching constraint, memory as its address), constants,
   symbols (an object's address, a function's table index, `call` of a
   function operand and of a function by name), asm goto, the vector,
   atomic, bulk-memory, sign-extension, saturating and multivalue
   instructions, and file-scope asm functions (calling C, and called by C)
   and a naked function. At -O0 and -O2 on wasm32 and wasm64. */
#include "host.h"

#if __AXIOM_PTR_WIDTH__ == 64
#define PCONST "i64.const"
#define PADD "i64.add"
#else
#define PCONST "i32.const"
#define PADD "i32.add"
#endif

typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

static int add(int a, int b) {
    int r;
    __asm__("local.get %1\n\tlocal.get %2\n\ti32.add\n\tlocal.set %0" : "=r"(r) : "r"(a), "r"(b));
    return r;
}

static double fmul(double a, double b) {
    double r;
    __asm__("(local.set %[r] (f64.mul (local.get %[a]) (local.get %[b])))" : [r] "=r"(r) : [a] "r"(a), [b] "r"(b));
    return r;
}

static long long shl(long long x) {
    __asm__("local.get %0 i64.const %1 i64.shl local.set %0" : "+r"(x) : "i"(3));
    return x;
}

static int tied(int x) {
    int r;
    __asm__("(local.set %0 (i32.mul (local.get %0) (i32.const -2)))" : "=r"(r) : "0"(x));
    return r;
}

static unsigned char narrow(signed char c, unsigned short s) {
    unsigned char r;
    /* (narrow inputs are their C values, extended to i32) */
    __asm__("local.get %1 local.get %2 i32.add local.set %0" : "=r"(r) : "r"(c), "r"(s));
    return r;
}

static int g = 41;
static long long big = 0x100000000LL;

static void incr(int *p) {
    __asm__("local.get %0 local.get %0 i32.load i32.const 1 i32.add i32.store" : "+m"(*p));
}

static long long load_off(void) {
    long long r;
    __asm__("local.get %1 i64.load offset=0 align=8 local.set %0" : "=r"(r) : "m"(big));
    return r;
}

static int twice(int x) { return 2 * x; }
int c_cube(int x) { return x * x * x; }

static int call_op(int x) {
    int r;
    __asm__("local.get %1 call %2 local.set %0" : "=r"(r) : "r"(x), "s"(twice));
    return r;
}

static int call_name(int x) {
    int r;
    __asm__("(local.set %0 (call $c_cube (local.get %1)))" : "=r"(r) : "r"(x));
    return r;
}

static int call_ptr(int (*f)(int), int x) {
    int r;
    /* (a function pointer is a table index, an i32 also on wasm64) */
#if __AXIOM_PTR_WIDTH__ == 64
    __asm__("local.get %2 local.get %1 i32.wrap_i64 call_indirect (param i32) (result i32) local.set %0"
            : "=r"(r) : "r"(f), "r"(x));
#else
    __asm__("local.get %2 local.get %1 call_indirect (param i32) (result i32) local.set %0" : "=r"(r) : "r"(f), "r"(x));
#endif
    return r;
}

static int *addr_of_g(void) {
    int *p;
    __asm__(PCONST " %1 local.set %0" : "=r"(p) : "i"(&g));
    return p;
}

static int (*fn_of_twice(void))(int) {
    int (*p)(int);
    __asm__(PCONST " %1 local.set %0" : "=r"(p) : "i"(twice));
    return p;
}

static int sign(int x) {
    asm goto("local.get %0 i32.const 0 i32.lt_s br_if %l1 local.get %0 i32.eqz br_if %l[zero]" : : "r"(x) : : neg, zero);
    return 1;
neg:
    return -1;
zero:
    return 0;
}

static int count_until(int limit) {
    int n = 0;
    for (;;) {
        n++;
        asm goto("(br_if %l[done] (i32.ge_s (local.get %0) (local.get %1)))" : : "r"(n), "r"(limit) : : done);
    }
done:
    return n;
}

static int sum_to(int n) {
    int s;
    __asm__("i32.const 0 local.set %0\n"
            "block $out\n"
            "  loop $top\n"
            "    local.get %1 i32.eqz br_if $out\n"
            "    (local.set %0 (i32.add (local.get %0) (local.get %1)))\n"
            "    (local.set %1 (i32.sub (local.get %1) (i32.const 1)))\n"
            "    br $top\n"
            "  end\n"
            "end"
            : "=&r"(s), "+r"(n));
    return s;
}

static int pick(int c, int a, int b) {
    int r;
    __asm__("(local.set %0 (select (local.get %2) (local.get %3) (local.get %1)))" : "=r"(r) : "r"(c), "r"(a), "r"(b));
    return r;
}

static v4si vadd(v4si a, v4si b) {
    v4si r;
    __asm__("local.get %1 local.get %2 i32x4.add local.set %0" : "=r"(r) : "r"(a), "r"(b));
    return r;
}

static int lane2(v4si a) {
    int r;
    __asm__("(local.set %0 (i32x4.extract_lane 2 (local.get %1)))" : "=r"(r) : "r"(a));
    return r;
}

static v4sf vconst(void) {
    v4sf r;
    __asm__("v128.const f32x4 1.5 -0x1.8p-1 0x1p+3 3e2 local.set %0" : "=r"(r));
    return r;
}

static v4si interleave(v4si a, v4si b) {
    v4si r;
    __asm__("local.get %1 local.get %2 i8x16.shuffle 0 1 2 3 16 17 18 19 4 5 6 7 20 21 22 23 local.set %0"
            : "=r"(r) : "r"(a), "r"(b));
    return r;
}

static int xchg(int *p, int v) {
    int old;
    __asm__ volatile("local.get %1 local.get %2 i32.atomic.rmw.xchg local.set %0" : "=r"(old), "+m"(*p) : "r"(v));
    return old;
}

static long long multi(int a) {
    long long r;
    __asm__("block (result i32 i64) local.get %1 i64.const 7 end local.set %0 i64.extend_i32_s local.get %0 i64.add "
            "local.set %0"
            : "=r"(r) : "r"(a));
    return r;
}

static int ext8(long long x) {
    int r;
    __asm__("local.get %1 i32.wrap_i64 i32.extend8_s local.set %0" : "=r"(r) : "r"(x));
    return r;
}

static int sat(double d) {
    int r;
    __asm__("local.get %1 i32.trunc_sat_f64_s local.set %0" : "=r"(r) : "r"(d));
    return r;
}

static void fill(char *p, size_t n) {
    __asm__("local.get %0 i32.const 65 local.get %1 memory.fill" : : "r"(p), "r"(n) : "memory");
}

static double fconst(void) {
    double r;
    __asm__("f64.const -0x1.fffffffffffffp+1023 local.set %0" : "=r"(r));
    return r;
}

static int uniq(void) {
    int r;
    __asm__("(local.set %0 (i32.const %=)) (local.set %0 (i32.sub (local.get %0) (i32.const %=)))" : "=r"(r));
    return r;
}

/* file-scope asm: functions (module fields), C's names by declaration */
__asm__("(func $asm_add (param $a i32) (param $b i32) (result i32)\n"
        "  (i32.add (local.get $a) (local.get $b)))\n"
        "(func $asm_cube_plus1 (param i32) (result i32)\n"
        "  local.get 0 call $c_cube i32.const 1 i32.add)\n"
        "(func $asm_sum (param $n i32) (result i32) (local $s i32)\n"
        "  block $done\n"
        "    loop $top\n"
        "      local.get $n i32.eqz br_if $done\n"
        "      (local.set $s (i32.add (local.get $s) (local.get $n)))\n"
        "      (local.set $n (i32.sub (local.get $n) (i32.const 1)))\n"
        "      br $top\n"
        "    end\n"
        "  end\n"
        "  local.get $s)\n");
__asm__("(func $asm_later (result i32) (call $asm_add (i32.const 30) (i32.const 12)))");
int asm_add(int, int);
int asm_cube_plus1(int);
int asm_sum(int);
int asm_later(void);

__attribute__((naked)) int naked_mul(int a, int b) {
    __asm__("local.get 0 local.get 1 i32.mul");
}

int main(void) {
    print_i64(add(2, 3));
    print_f64(fmul(1.5, 4));
    print_i64(shl(5));
    print_i64(tied(21));
    print_i64(narrow(-1, 300));
    incr(&g);
    print_i64(g);
    print_i64(load_off());
    print_i64(call_op(21));
    print_i64(call_name(3));
    print_i64(call_ptr(twice, 50));
    print_i64(*addr_of_g());
    print_i64(fn_of_twice()(8));
    print_i64(sign(-5));
    print_i64(sign(0));
    print_i64(sign(7));
    print_i64(count_until(9));
    print_i64(sum_to(100));
    print_i64(pick(1, 10, 20) + pick(0, 10, 20));
    v4si a = {1, 2, 3, 4}, b = {10, 20, 30, 40};
    v4si c = vadd(a, b);
    print_i64(c[0] + c[1] * 100 + c[3] * 10000);
    print_i64(lane2(c));
    v4sf f = vconst();
    print_f64(f[0]);
    print_f64(f[1]);
    print_f64(f[2]);
    print_f64(f[3]);
    v4si s = interleave(a, b);
    print_i64(s[0] * 1000 + s[1] * 100 + s[2] * 10 + s[3]);
    int x = 5;
    print_i64(xchg(&x, 9));
    print_i64(x);
    print_i64(multi(3));
    print_i64(ext8(0x1ff));
    print_i64(sat(1e20));
    char buf[6] = {0};
    fill(buf, 5);
    print_str(buf, 5);
    print_bool(fconst() < -1e308);
    print_i64(uniq());
    print_i64(asm_add(3, 4));
    print_i64(asm_cube_plus1(5));
    print_i64(asm_sum(10));
    print_i64(asm_later());
    print_i64(naked_mul(6, 7));
    return 0;
}
/* expect: 5 */
/* expect: 6 */
/* expect: 40 */
/* expect: -42 */
/* expect: 43 */
/* expect: 42 */
/* expect: 4294967296 */
/* expect: 42 */
/* expect: 27 */
/* expect: 100 */
/* expect: 42 */
/* expect: 16 */
/* expect: -1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 9 */
/* expect: 5050 */
/* expect: 30 */
/* expect: 442211 */
/* expect: 33 */
/* expect: 1.5 */
/* expect: -0.75 */
/* expect: 8 */
/* expect: 300 */
/* expect: 2040 */
/* expect: 5 */
/* expect: 9 */
/* expect: 10 */
/* expect: -1 */
/* expect: 2147483647 */
/* expect: AAAAA */
/* expect: true */
/* expect: 0 */
/* expect: 7 */
/* expect: 126 */
/* expect: 55 */
/* expect: 42 */
/* expect: 42 */
