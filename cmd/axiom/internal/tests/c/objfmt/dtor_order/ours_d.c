/* This compiler's object: constructors and destructors of priorities, in
   .CRT$XC* and .CRT$XT* sections whose names order them as the C runtime
   runs them (forward), and in .init_array/.fini_array elsewhere. */
#include <stdio.h>
__attribute__((constructor(200))) static void c200(void) { puts("ctor 200"); }
__attribute__((constructor)) static void cdef(void) { puts("ctor default"); }
__attribute__((constructor(150))) static void c150(void) { puts("ctor 150"); }
__attribute__((destructor(150))) static void d150(void) { puts("dtor 150"); }
__attribute__((destructor)) static void ddef(void) { puts("dtor default"); }
__attribute__((destructor(200))) static void d200(void) { puts("dtor 200"); }
int ours_d(void) { return 7; }
