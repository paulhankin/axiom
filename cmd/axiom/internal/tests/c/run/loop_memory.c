/* Loads moved out of loops and globals kept in registers across them (-O2):
   only where nothing in the loop may write what is read, and stored back
   on every way out; where a store does alias (a pointer to the same global
   or array, a call that writes it, volatile), every access stays. */
#include "host.h"

typedef struct { int n; unsigned d[16]; } num;

static num A, B;
static int G, H;
static int steps;
static volatile int vsteps;
static int arr[32];

/* the loop's bound and a->d[i] read once: t is a local array */
static unsigned long long local_t(int i) {
    unsigned t[40] = { 0 };
    unsigned long long carry = 0;
    for (int j = 0; j < B.n; j++) {
        unsigned long long p = (unsigned long long)A.d[i] * B.d[j] + t[i + j] + carry;
        t[i + j] = (unsigned)p;
        carry = p >> 32;
    }
    unsigned long long h = carry;
    for (int k = 0; k < 40; k++) h = h * 31 + t[k];
    return h;
}

/* t may be A's own digits: the load of A.d[i] stays in the loop */
static void maybe_same(unsigned *t, int i) {
    unsigned long long carry = 0;
    for (int j = 0; j < B.n; j++) {
        unsigned long long p = (unsigned long long)A.d[i] * B.d[j] + t[j] + carry;
        t[j] = (unsigned)p;
        carry = p >> 32;
    }
}

/* a counter in a global: kept in a register, stored back on each exit */
static int count(int n, int stop) {
    int s = 0;
    for (int i = 0; i < n; i++) {
        steps++;
        if (arr[i & 31] == stop) return s;
        s += arr[i & 31];
    }
    return s;
}

/* the exit before the counter's first store */
static int count_late(int n, int stop) {
    int s = 0;
    for (int i = 0; i < n; i++) {
        if (arr[i & 31] == stop) break;
        steps += 2;
        s += i;
    }
    return s;
}

/* a pointer that is (or is not) the global: every store through it is seen */
static void through(int *p, int n) {
    for (int i = 0; i < n; i++) {
        G += 1;
        *p += G;
    }
}

static void bump(void) { G += 100; }

/* a call in the loop writes the global */
static void with_call(int n) {
    for (int i = 0; i < n; i++) {
        G += 1;
        if (i == 2) bump();
    }
}

/* volatile: each store stays */
static void vol(int n) {
    for (int i = 0; i < n; i++) vsteps++;
}

/* a load from a global that a store in the loop may write (p = &arr[k]) */
static int reload(int *p, int n) {
    int s = 0;
    for (int i = 0; i < n; i++) {
        s += arr[3];
        *p = s;
    }
    return s;
}

/* t passed to a function in the loop */
static void fill(unsigned *t, int j) { t[j] = (unsigned)j * 3u; }
static unsigned passed(int n) {
    unsigned t[8] = { 0 };
    unsigned s = 0;
    for (int j = 0; j < n; j++) {
        fill(t, j & 7);
        s += t[j & 7] + t[0];
    }
    return s;
}

static volatile int vk[4] = { 9, 0, 7, 5 };

int main(void) {
    A.n = 16;
    B.n = vk[0];
    for (int i = 0; i < 16; i++) {
        A.d[i] = (unsigned)i * 2654435761u;
        B.d[i] = (unsigned)i * 40503u + 7u;
    }
    for (int i = 0; i < 32; i++) arr[i] = i * 5 % 11;
    unsigned long long h = 0;
    for (int i = 0; i < 16; i++) h = h * 7 + local_t(i);
    print_u64(h);
    maybe_same(A.d, 3);
    maybe_same(A.d + 1, 2);
    h = 0;
    for (int i = 0; i < 16; i++) h = h * 1000003 + A.d[i];
    print_u64(h);
    print_i64(count(100, 9));
    print_i64(steps);
    print_i64(count(vk[1], 9));
    print_i64(steps);
    print_i64(count(40, 100));
    print_i64(steps);
    print_i64(count_late(50, 0));
    print_i64(steps);
    print_i64(count_late(50, 10));
    print_i64(steps);
    G = 1;
    through(&G, vk[2]);
    print_i64(G);
    H = 1;
    through(&H, vk[2]);
    print_i64(G);
    print_i64(H);
    G = 0;
    with_call(vk[3]);
    print_i64(G);
    vol(vk[3]);
    print_i64(vsteps);
    print_i64(reload(&arr[3], vk[2]));
    print_i64(arr[3]);
    print_i64(reload(&arr[4], vk[2]));
    print_i64(passed(vk[0] + 3));
    return 0;
}
/* expect: 14785253131100422272 */
/* expect: 2390047180793962249 */
/* expect: 19 */
/* expect: 5 */
/* expect: 0 */
/* expect: 5 */
/* expect: 200 */
/* expect: 45 */
/* expect: 0 */
/* expect: 45 */
/* expect: 1 */
/* expect: 49 */
/* expect: 382 */
/* expect: 389 */
/* expect: 2703 */
/* expect: 105 */
/* expect: 5 */
/* expect: 256 */
/* expect: 256 */
/* expect: 1792 */
/* expect: 102 */
