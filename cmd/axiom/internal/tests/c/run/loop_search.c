/* flags: --c-std=c11 */
/* Loops that search for an element equal to (or different from) a value
   run first as vector searches over aligned 16-byte vectors (vectorize.ax:
   search_loop), then the scalar loop finds the element: bytes, ints and
   shorts, by index and by pointer, at every start offset (the first vector
   beginning before the first element) and every count from 0, with and
   without a match, whose index is the loop's. */
#include "host.h"
static unsigned long find(const unsigned char *p, unsigned long n, unsigned char c) {
    unsigned long i;
    for (i = 0; i < n; i++)
        if (p[i] == c)
            break;
    return i;
}
static int findi(const int *p, int n, int c) {
    for (int i = 0; i < n; i++)
        if (p[i] != c)
            return i;
    return -1;
}
static const short *finds(const short *p, const short *e, short c) {
    while (p != e) {
        if (*p == c)
            return p;
        p++;
    }
    return 0;
}
static unsigned char buf[300];
static int ib[100];
static short sb[100];
int main(void) {
    unsigned long long h = 0;
    for (int i = 0; i < 300; i++) buf[i] = (unsigned char)(i * 7 % 251);
    for (int i = 0; i < 100; i++) { ib[i] = 5; sb[i] = (short)(i * 3); }
    ib[77] = 6;
    for (int off = 0; off < 20; off++)
        for (int n = 0; n < 90; n++)
            for (int c = 0; c < 256; c += 37)
                h = h * 31 + find(buf + off, n, (unsigned char)c);
    for (int off = 0; off < 10; off++)
        for (int n = 0; n < 90; n++)
            h = h * 31 + (unsigned)findi(ib + off, n, 5);
    for (int off = 0; off < 10; off++)
        for (int n = 0; n < 90; n++) {
            const short *r = finds(sb + off, sb + off + n, 60);
            h = h * 31 + (r ? (unsigned long long)(r - sb) : 999);
        }
    print_u64(h);
    return 0;
}
/* expect: 7074238613108623037 */
