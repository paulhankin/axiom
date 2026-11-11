/* host_native.c - the output functions of host.h for the native C tests:
   those of ../native_rt.c, without its entry point (a C program's start-up
   is the compiler's). Compiled by the system's C compiler. */
#define main axiom_tests_native_rt_main
#include "../native_rt.c"
#undef main
void run(void) {}

/* (unbuffered, as native_rt.c's main makes it: output before a crash is kept) */
__attribute__((constructor)) static void unbuffered(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
}
