/* flags: --c-std=gnu99 */
/* The GNU dialects' statements and expressions: statement expressions,
   local labels, __auto_type, cleanups on every way out of a scope,
   computed goto through a static table, case ranges, `?:` with its middle
   operand left out, and the keywords' other spellings. */
#include "host.h"

#define MAX(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); _a > _b ? _a : _b; })
#define FIND(arr, n, v) ({ __label__ found; int _i; \
    for (_i = 0; _i < (n); _i++) if ((arr)[_i] == (v)) goto found; \
    _i = -1; found: _i; })

static int log_[16];
static int nlog;
static void note(int *p) { log_[nlog++] = *p; }

struct pt { int x, y; };

static int cleanup_paths(int how) {
    int a __attribute__((cleanup(note))) = 1;
    for (int i = 0; i < 2; i++) {
        int b __attribute__((cleanup(note))) = 10 + i;
        if (how == 0 && i == 1)
            break;
        if (how == 1)
            goto out;
        if (how == 2)
            return 100;
    }
out:;
    int c __attribute__((cleanup(note))) = 3;
    return how;
}

static int interp(const unsigned char *code) {
    static const void *ops[] = { &&op_inc, &&op_dbl, &&op_end };
    int acc = 0;
    const unsigned char *pc = code;
    goto *ops[*pc++];
op_inc:
    acc += 1;
    goto *ops[*pc++];
op_dbl:
    acc *= 2;
    goto *ops[*pc++];
op_end:
    return acc;
}

static int classify(signed char c) {
    switch (c) {
    case -128 ... -1: return -1;
    case '0' ... '9': return 1;
    case 'a' ... 'z':
    case 'A' ... 'Z': return 2;
    case 1000 - 1000: return 0;
    default: return 3;
    }
}

static int big_range(unsigned x) {
    switch (x) {
    case 0 ... 100000: return 1;
    case 100001 ... 4000000000u: return 2;
    default: return 3;
    }
}

static int calls;
static int next(void) { return ++calls; }

int (renamed)(int) __asm__("real_name");
int real_name(int x) { return x + 1000; }

static __inline__ int inl(int __const x) { return x; }

int main(void) {
    int u = 3, w = 9;
    print_i64(MAX(u, w) + MAX(2.5, 1.5) * 0 + ({ int t = u * w; t; }));
    struct pt p = ({ struct pt q = { 4, 5 }; q; });
    print_i64(p.x * 10 + p.y);
    int arr[] = { 5, 7, 9 };
    print_i64(FIND(arr, 3, 9) * 10 + FIND(arr, 3, 4));
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += ({ if (i == 3) continue; if (i == 7) break; i; });
    }
    print_i64(sum);
    for (int how = 0; how < 4; how++) {
        nlog = 0;
        int r = cleanup_paths(how);
        int code = 0;
        for (int k = 0; k < nlog; k++)
            code = code * 100 + log_[k];
        print_i64((long long)r * 100000000 + code);
    }
    unsigned char prog[] = { 0, 0, 1, 0, 1, 2 };
    print_i64(interp(prog));
    print_i64(classify(-5) * 1000 + classify('7') * 100 + classify('Q') * 10 + classify(0) + classify('!') * 10000);
    print_i64(big_range(5) * 100 + big_range(3000000000u) * 10 + big_range(4000000001u));
    calls = 0;
    int v = next() ?: 50;
    print_i64(v * 10 + calls);
    print_i64((0 ?: 7) + (char)(4 ?: 9));
    __typeof__(u) tu = 4;
    typeof(w) tw = 5;
    __signed__ char sc = -1;
    __volatile__ int vol = 2;
    int nv = vol;
    int *__restrict rp = &nv;
    print_i64(tu + tw + sc + *rp + inl(1));
    struct { char c; double d; } sd;
    print_i64(__alignof__(sd.d) * 100 + __alignof__ sd + __alignof__(int) * 0);
    __extension__ long long ext = __extension__ 0x100000000LL;
    print_i64(ext);
    print_i64(renamed(5));
    print_i64(0b1011 + '\e');
    print_i64(__COUNTER__ + __COUNTER__ * 10);
    print_str(__func__, 4);
    print_str(__FUNCTION__, 4);
    print_str(__PRETTY_FUNCTION__, 4);
    return 0;
}
/* expect: 36 */
/* expect: 45 */
/* expect: 19 */
/* expect: 18 */
/* expect: 10110301 */
/* expect: 100100301 */
/* expect: 10000001001 */
/* expect: 310110301 */
/* expect: 10 */
/* expect: 29120 */
/* expect: 123 */
/* expect: 11 */
/* expect: 11 */
/* expect: 11 */
/* expect: 808 */
/* expect: 4294967296 */
/* expect: 1005 */
/* expect: 38 */
/* expect: 10 */
/* expect: main */
/* expect: main */
/* expect: main */
