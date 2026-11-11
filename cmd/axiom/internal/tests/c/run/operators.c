/* Operators: comma, ?: and its typing, compound assignments with their
   conversions, ++/-- on integers, floats and pointers, logical operators'
   short circuit, comparison results, casts, sizeof and _Alignof. */
#include "host.h"

static int calls;
static int touch(int v) { calls++; return v; }

int main(void) {
    int i = 5, j;
    unsigned char c = 250;
    short s = 1000;
    double d = 1.5;
    float f = 2.5f;
    int a[4] = { 1, 2, 3, 4 };
    int *p = a;
    char *cp;
    long long ll;
    j = (i++, i * 2);
    print_i64(j);
    print_i64(i);
    print_i64(i > 3 ? 10 : 20);
    print_f64(i > 3 ? 1 : 2.5);
    print_i64(sizeof(i ? c : s));
    print_i64(sizeof(i ? 'a' : 1.0));
    c += 10;
    print_i64(c);
    c -= 300;
    print_i64(c);
    s *= 100;
    print_i64(s);
    i = 7;
    i /= 2.0;
    print_i64(i);
    i = 100;
    i %= 7;
    print_i64(i);
    i <<= 4;
    print_i64(i);
    i >>= 2;
    print_i64(i);
    i &= 6;
    print_i64(i);
    i |= 9;
    print_i64(i);
    i ^= 3;
    print_i64(i);
    d *= 3;
    print_f64(d);
    d++;
    print_f64(d);
    print_f64(d--);
    print_f64(--d);
    f += 0.25;
    print_f64(f);
    p += 2;
    print_i64(*p);
    print_i64(*p++);
    print_i64(*p);
    print_i64(*--p);
    p -= 1;
    print_i64(*p);
    c = 255;
    c++;
    print_i64(c);
    i = 0;
    print_i64(touch(0) && touch(1));
    print_i64(calls);
    print_i64(touch(1) || touch(1));
    print_i64(calls);
    print_i64(touch(1) && touch(2));
    print_i64(calls);
    print_i64(touch(0) || touch(0));
    print_i64(calls);
    print_i64((3 < 4) + (4 < 3) + (5 == 5) + (5 != 5) * 10);
    print_i64(sizeof(3 < 4));
    print_i64(!!7);
    print_i64((char)0x1234);
    print_i64((unsigned char)-1);
    print_i64((short)0x18000);
    print_i64((int)(unsigned char)200);
    print_f64((float)0.1);
    print_f64((double)(float)16777217.0);
    print_i64((long long)-1.5);
    ll = (long long)(void *)0;
    print_i64(ll);
    cp = (char *)a;
    print_i64(*(int *)(cp + 4));
    print_i64((int)sizeof(char[10]));
    print_i64(sizeof(int[3][2]));
    print_i64(_Alignof(char) + _Alignof(short) * 10 + _Alignof(int) * 100 + _Alignof(double) * 1000);
    print_i64(_Alignof(struct { char c; int i; }));
    print_i64(sizeof a / sizeof a[0]);
    print_i64(sizeof(a + 0) == sizeof(int *));
    j = 1;
    print_i64(sizeof(j++));
    print_i64(j);
    i = 10;
    j = i++ + i;
    print_i64(j);
    print_i64(-(-i));
    print_i64(- -i);
    print_i64(~0u >> 28);
    i = 3;
    print_i64(i = 8);
    print_i64(i);
    print_i64((i += 2) * 2);
    {
        int x = 1, y = 2, z = 3;
        x = y = z;
        print_i64(x + y + z);
        print_i64(x == y == 1);
        print_i64(1 ? 2 ? 3 : 4 : 5);
        print_i64(0 ? 1 : 0 ? 2 : 3);
    }
    return 0;
}

/* expect: 12 */
/* expect: 6 */
/* expect: 10 */
/* expect: 1 */
/* expect: 4 */
/* expect: 8 */
/* expect: 4 */
/* expect: 216 */
/* expect: -31072 */
/* expect: 3 */
/* expect: 2 */
/* expect: 32 */
/* expect: 8 */
/* expect: 0 */
/* expect: 9 */
/* expect: 10 */
/* expect: 4.5 */
/* expect: 5.5 */
/* expect: 5.5 */
/* expect: 3.5 */
/* expect: 2.75 */
/* expect: 3 */
/* expect: 3 */
/* expect: 4 */
/* expect: 3 */
/* expect: 2 */
/* expect: 0 */
/* expect: 0 */
/* expect: 1 */
/* expect: 1 */
/* expect: 2 */
/* expect: 1 */
/* expect: 4 */
/* expect: 0 */
/* expect: 6 */
/* expect: 2 */
/* expect: 4 */
/* expect: 1 */
/* expect: 52 */
/* expect: 255 */
/* expect: -32768 */
/* expect: 200 */
/* expect: 0.10000000149011612 */
/* expect: 16777216 */
/* expect: -1 */
/* expect: 0 */
/* expect: 2 */
/* expect: 10 */
/* expect: 24 */
/* expect: 8421 */
/* expect: 4 */
/* expect: 4 */
/* expect: 1 */
/* expect: 4 */
/* expect: 1 */
/* expect: 21 */
/* expect: 11 */
/* expect: 11 */
/* expect: 15 */
/* expect: 8 */
/* expect: 8 */
/* expect: 20 */
/* expect: 9 */
/* expect: 1 */
/* expect: 3 */
/* expect: 3 */
