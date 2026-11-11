/* flags: --c-std=c23 */
/* The types of C23's literals and typeof: true and false are bool; u8
   character constants are char8_t (unsigned char) and u8 strings arrays of
   char8_t (not char); binary constants and digit separators take the types
   of other octal and hexadecimal constants (int, unsigned, long ...);
   typeof keeps an operand's qualifiers and array type, typeof_unqual drops
   them (an array's with its elements'). */
#include "host.h"

#define T(x) _Generic((x), bool: 1, char: 2, signed char: 3, unsigned char: 4, int: 5, unsigned: 6, \
    long long: 7, unsigned long long: 8, unsigned char *: 9, char *: 10, int *: 11, default: 0)

int main(void) {
    const int ci = 3;
    int arr[5];
    const int carr[2] = { 1, 2 };
    print_i64(T(true));
    print_i64(T(false));
    print_i64(true + true);
    print_i64(T(true + 0));
    print_i64(T(u8'a'));
    print_i64(u8'\xff');
    print_i64(T(u8"x"));
    print_i64(sizeof(u8"abc"));
    print_i64(T(0b101));
    print_i64(0b101);
    print_i64(T(0b11111111111111111111111111111111));
    print_i64(T(1'000'000));
    print_i64(1'000'000);
    print_i64(0x7FFF'FFFF);
    print_i64(T(0xFFFF'FFFF'FFFF'FFFFull));
    print_i64(T(07'7));
    print_i64(07'7);
    typeof(ci) t1 = 4;
    typeof_unqual(ci) t2 = 5;
    t2++;
    print_i64(_Generic(&t1, const int *: 1, default: 0));
    print_i64(_Generic(&t2, int *: 1, default: 0));
    print_i64(sizeof(typeof(arr)) / sizeof(int));
    print_i64(sizeof(typeof("abcd")));
    print_i64(_Generic((typeof_unqual(carr) *)0, int (*)[2]: 1, default: 0));
    print_i64(_Generic((typeof(carr) *)0, const int (*)[2]: 1, default: 0));
    typeof(int (*)(void)) fp = 0;
    print_i64(fp == nullptr);
    typeof(1wb) b2 = 1;
    print_i64(sizeof b2);
    typeof(ci + 1) plain = 7;
    plain++;
    print_i64(plain);
    typeof(main) *mp = main;
    print_i64(mp == main);
    const struct { int m; double d; } cs = { 1, 2.0 };
    print_i64(_Generic((typeof(cs.m) *)0, const int *: 1, default: 0) +
              _Generic((typeof_unqual(cs.m) *)0, int *: 2, default: 0));
    int x = 0;
    print_i64(_Generic((typeof(x = 1.5) *)0, int *: 1, default: 0));
    print_i64(x);
    print_i64(_Generic((typeof((0, arr)) *)0, int **: 1, default: 0));
    print_i64(_Generic((typeof(x ? 1 : 2.0f) *)0, float *: 1, default: 0));
    print_i64(_Generic((typeof((long)x) *)0, long *: 1, default: 0));
    print_i64(_Generic((typeof(_Generic(1, int: 'c')) *)0, int *: 1, default: 0));
    return 0;
}
/* expect: 1 */
/* expect: 1 */
/* expect: 2 */
/* expect: 5 */
/* expect: 4 */
/* expect: 255 */
/* expect: 9 */
/* expect: 4 */
/* expect: 5 */
/* expect: 5 */
/* expect: 6 */
/* expect: 5 */
/* expect: 1000000 */
/* expect: 2147483647 */
/* expect: 8 */
/* expect: 5 */
/* expect: 63 */
/* expect: 1 */
/* expect: 1 */
/* expect: 5 */
/* expect: 5 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 8 */
/* expect: 1 */
/* expect: 3 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
