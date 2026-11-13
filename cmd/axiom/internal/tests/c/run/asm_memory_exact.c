/* flags: --c-std=gnu17 */
/* native-only */
/* Asm statements with memory operands and no "memory" clobber read and write
   exactly their objects: at every optimization level a store to the object
   before the statement reaches it, a load after it sees what it wrote (in a
   loop, of a local, of an array element, through a restrict pointer), while
   other objects keep their values across it. */
#include "host.h"

#if defined(__AXIOM_ASM_X86_64__)
#define READ(t, x) __asm__("movl %1, %0" : "=r"(t) : "m"(x))
#define WRITE(x, v) __asm__("movl %1, %0" : "=m"(x) : "r"(v))
#define BUMP(x) __asm__("addl $1, %0" : "+m"(x))
#elif defined(__AXIOM_ASM_AARCH64__)
#define READ(t, x) __asm__("ldr %w0, %1" : "=r"(t) : "m"(x))
#define WRITE(x, v) __asm__("str %w1, %0" : "=m"(x) : "r"(v))
#define BUMP(x) __asm__("ldr w9, %0\n\tadd w9, w9, #1\n\tstr w9, %0" : "+m"(x) : : "x9")
#else
#define READ(t, x) ((t) = (x))
#define WRITE(x, v) ((x) = (v))
#define BUMP(x) ((x) += 1)
#endif

int g, h;
int arr[8];

static int stored_then_read(void) {
    int t;
    g = 5;
    READ(t, g);
    g = 6;
    return t * 10 + g;  /* 56 */
}

static int written_then_loaded(void) {
    g = 1;
    h = 40;
    WRITE(g, 9);
    return g + h;  /* 49 */
}

static int loop_sum(int n) {
    int s = 0, t;
    for (int i = 0; i < n; i++) {
        arr[i & 7] = i;
        READ(t, arr[i & 7]);
        s += t;
    }
    return s;
}

static int local_bumped(int k) {
    int loc = k;
    for (int i = 0; i < 3; i++) BUMP(loc);
    return loc;
}

static int restricted(int *restrict p, int *restrict q) {
    *p = 1;
    *q = 2;
    BUMP(*p);
    return *p * 10 + *q;  /* 22 */
}

int main(void) {
    print_i64(stored_then_read());
    print_i64(written_then_loaded());
    print_i64(loop_sum(100));
    print_i64(local_bumped(4));
    int a = 0, b = 0;
    print_i64(restricted(&a, &b));
    print_i64(a);
    return 0;
}
/* expect: 56 */
/* expect: 49 */
/* expect: 4950 */
/* expect: 7 */
/* expect: 22 */
/* expect: 2 */
