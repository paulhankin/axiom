/* flags: --c-std=gnu11 */
#include "host.h"
struct __attribute__((packed)) P { char c; int i; short s; };
struct A { char c; int i __attribute__((aligned(8))); };
struct __attribute__((packed)) B { char c; int b:20; int d:12; char e; };
#pragma pack(push, 2)
struct Q { char c; int x; short y; };
#pragma pack(pop)
static int cnt;
static void cleanup_int(int *p) { cnt += *p; }
static int f(int x) {
    int r = ({ int t = x * 2; t + 1; });
    return r ?: 99;
}
static int ranges(int x) {
    switch (x) {
    case 1 ... 3: return 10;
    case 4: return 20;
    case 100 ... 1000: return 30;
    default: return 0;
    }
}
static int dispatch(int k) {
    static void *tbl[] = { &&a, &&b, &&c };
    int acc = 0;
    goto *tbl[k];
a: acc += 1;
b: acc += 10;
c: acc += 100;
    return acc;
}
int main(void) {
    struct P p = { 1, 0x12345678, 7 };
    print_i64(sizeof(struct P));
    print_i64(p.i);
    p.i += 1;
    print_i64(p.i);
    print_i64(sizeof(struct A));
    print_i64(_Alignof(struct A));
    struct B b = {0};
    b.b = 0x12345; b.d = 0x7ff; b.e = 9;
    print_i64(sizeof(struct B));
    print_i64(b.b);
    print_i64(b.d);
    print_i64(b.e);
    print_i64(sizeof(struct Q));
    print_i64(f(3));
    print_i64(f(0) ?: 5);
    print_i64(ranges(2) + ranges(4) + ranges(500) + ranges(5000));
    print_i64(dispatch(0) + dispatch(1) * 1000 + dispatch(2) * 1000000);
    {
        int v __attribute__((cleanup(cleanup_int))) = 5;
        int w __attribute__((cleanup(cleanup_int))) = 7;
        (void)v; (void)w;
    }
    print_i64(cnt);
    int arr[6] = { [0 ... 2] = 4, [4 ... 5] = 9 };
    print_i64(arr[0] + arr[2] + arr[3] + arr[5]);
    void *vp = arr;
    vp = vp + 4;
    print_i64(*(int *)vp);
    print_i64(sizeof(void));
    return 0;
}
/* expect: 7 */
/* expect: 305419896 */
/* expect: 305419897 */
/* expect: 16 */
/* expect: 8 */
/* expect: 6 */
/* expect: 74565 */
/* expect: 2047 */
/* expect: 9 */
/* expect: 8 */
/* expect: 7 */
/* expect: 1 */
/* expect: 60 */
/* expect: 100110111 */
/* expect: 12 */
/* expect: 17 */
/* expect: 4 */
/* expect: 1 */
