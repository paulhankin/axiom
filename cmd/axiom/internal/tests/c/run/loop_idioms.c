/* flags: --c-std=c11 */
/* Loops that copy or fill consecutive elements become memmove, memcpy and
   memset (idioms.ax). The results must be the loops': a forward copy whose
   destination starts inside its source repeats elements (the loop is kept
   for that case), one below or apart is memmove's; counts from every kind
   of test (< and != up, != and > down, signed and unsigned, none, wrapping
   in 8 bits); a fill of a value whose bytes differ stays a loop; float bits
   (a NaN's payload) are copied as they are; restrict-qualified parameters
   give a copy without the overlap check. Copies of every length up to 100
   bytes at every offset pair take the inline paths of memory operations of
   1 to 32 bytes and the long one. */
#include "host.h"
static void copy_bytes(void *dst, const void *src, unsigned long size) {
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (size--)
        *d++ = *s++;
}
static void copy_ints(int *d, const int *s, int n) {
    for (int i = 0; i < n; i++)
        d[i] = s[i];
}
static void copy_down(long long *d, const long long *s, long n) {
    for (long i = n; i != 0; i--)
        *d++ = *s++;
}
static void copy_gt(short *d, const short *s, int n) {
    for (int i = n; i > 0; i--)
        *d++ = *s++;
}
static void copy_restrict(int *restrict d, const int *restrict s, unsigned n) {
    for (unsigned i = 0; i < n; i++)
        d[i] = s[i];
}
static void copy_floats(float *d, const float *s, int n) {
    for (int i = 0; i < n; i++)
        d[i] = s[i];
}
static void fill_i(int *p, int n, int v) {
    for (int i = 0; i < n; i++)
        p[i] = v;
}
static void zero_ll(long long *p, unsigned n) {
    for (unsigned i = 0; i < n; i++)
        p[i] = 0;
}
static void fill_c(char *p, unsigned long a, unsigned long b, char c) {
    for (unsigned long i = a; i < b; i++)
        p[i] = c;
}
static void fill_m1(short *p, int a, int b) {
    for (int i = a; i < b; i++)
        p[i] = -1;
}
static void fill_ne(unsigned char *p, unsigned char a, unsigned char b) {
    for (unsigned char i = a; i != b; i++)
        p[i] = 0x5a;
}
static void fill_mixed(int *p, int n) {
    for (int i = 0; i < n; i++)
        p[i] = 0x01020304;
}
unsigned char buf[320];
static unsigned long long sum(void) {
    unsigned long long s = 0;
    for (int i = 0; i < 320; i++)
        s = s * 31 + buf[i];
    return s;
}
static void reset(void) {
    for (int i = 0; i < 320; i++)
        buf[i] = (unsigned char)(i * 7 + 3);
}
int main(void) {
    static const int offs[] = { 0, 1, 3, 8, 17, 40, 100 };
    static const int lens[] = { 0, 1, 2, 3, 4, 7, 8, 9, 15, 16, 17, 31, 32, 33, 64, 100 };
    unsigned long long h = 0;
    for (int a = 0; a < 7; a++)
        for (int b = 0; b < 7; b++)
            for (int l = 0; l < 16; l++) {
                int n = lens[l];
                reset();
                copy_bytes(buf + offs[a], buf + offs[b], n);
                h = h * 1000003 + sum();
                reset();
                copy_ints((int *)(buf + 4 * (offs[a] / 4 + 1)), (int *)(buf + 4 * (offs[b] / 4 + 1)), n / 2);
                h = h * 1000003 + sum();
                reset();
                copy_down((long long *)(buf + 8 * (offs[a] / 8 + 1)), (long long *)(buf + 8 * (offs[b] / 8 + 1)), n / 4);
                h = h * 1000003 + sum();
                reset();
                copy_gt((short *)(buf + 2 * (offs[a] / 2 + 1)), (short *)(buf + 2 * (offs[b] / 2 + 1)), n);
                h = h * 1000003 + sum();
            }
    print_u64(h);
    /* the smearing copy: the destination one element into the source */
    reset();
    copy_bytes(buf + 1, buf, 10);
    print_i64(buf[1] == 3 && buf[10] == 3 && buf[11] == (unsigned char)(11 * 7 + 3));
    reset();
    copy_ints((int *)(buf + 8), (int *)(buf + 4), 5);
    print_i64(((int *)buf)[6] == ((int *)buf)[1]);
    h = 0;
    for (int l = 0; l < 16; l++) {
        int n = lens[l];
        reset();
        fill_i((int *)(buf + 4), n / 2, -3 + l);
        h = h * 1000003 + sum();
        reset();
        zero_ll((long long *)(buf + 8), n / 4);
        h = h * 1000003 + sum();
        reset();
        fill_c((char *)buf, 3, 3 + n, (char)l);
        h = h * 1000003 + sum();
        reset();
        fill_c((char *)buf, 50, 3 + n, (char)l);
        h = h * 1000003 + sum();
        reset();
        fill_m1((short *)buf, 5, n);
        h = h * 1000003 + sum();
        reset();
        fill_m1((short *)buf, l, 5);
        h = h * 1000003 + sum();
        reset();
        fill_mixed((int *)(buf + 4), n / 2);
        h = h * 1000003 + sum();
    }
    print_u64(h);
    /* != in 8 bits: from 250 up to 4, wrapping */
    reset();
    fill_ne(buf, 250, 4);
    print_i64(buf[250] + buf[255] + buf[0] + buf[3] + (buf[4] != 0x5a) + (buf[249] != 0x5a));
    {
        static int src[40], dst[40];
        static float fs[4], fd[4];
        static const unsigned nan_bits = 0x7fa00123u;
        for (int i = 0; i < 40; i++)
            src[i] = i * i;
        copy_restrict(dst + 2, src, 30);
        print_i64(dst[2] + dst[31] + dst[1] + dst[32]);
        union { unsigned u; float f; } x, y;
        x.u = nan_bits;
        fs[1] = x.f;
        copy_floats(fd, fs, 4);
        y.f = fd[1];
        print_i64(y.u == nan_bits);
    }
    return 0;
}
/* expect: 13866082913162505499 */
/* expect: 1 */
/* expect: 1 */
/* expect: 661352158070972229 */
/* expect: 362 */
/* expect: 841 */
/* expect: 1 */
