#include <stdio.h>
__attribute__((constructor(150))) static void c150(void) { printf("ours 150\n"); }
__attribute__((constructor)) static void c0(void) { printf("ours\n"); }
__attribute__((destructor)) static void d0(void) { printf("ours dtor\n"); }
__attribute__((section("mysec"), used)) int sec_c = 33;
