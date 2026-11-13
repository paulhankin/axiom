#include <stdio.h>
__attribute__((constructor(200))) static void c200(void) { printf("sys 200\n"); }
__attribute__((constructor(101))) static void c101(void) { printf("sys 101\n"); }
__attribute__((constructor)) static void c0(void) { printf("sys\n"); }
__attribute__((destructor)) static void d0(void) { printf("sys dtor\n"); }
__attribute__((section("mysec"), used)) int sec_a = 11;
__attribute__((section("mysec"), used)) int sec_b = 22;
extern int __start_mysec[], __stop_mysec[];
int sec_sum(void) {
    int s = 0;
    for (int *p = __start_mysec; p < __stop_mysec; p++)
        s += *p;
    return s;
}
