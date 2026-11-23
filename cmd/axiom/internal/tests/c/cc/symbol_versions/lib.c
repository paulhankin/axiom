/* A library of two versions: f, call_foo and the old foo are LIB_1.0's, g and
   the new foo LIB_2.0's; names of C++ functions (as their mangled names) are
   chosen by their demangled names. */
int f(void) { return 1; }
int g(void) { return 2; }
int not_exported(void) { return 3; }

int foo_old(void) { return 10; }
int foo_new(void) { return 20; }
#ifndef ONE_VERSION
__asm__(".symver foo_old, foo@LIB_1.0\n"
        ".symver foo_new, foo@@LIB_2.0");
#endif
int foo(void);
int call_foo(void) { return foo() + 1; } /* the default version: foo_new */

int ns_f(int a) __asm__("_ZN2ns1fEi");
int ns_f(int a) { return a * 2; }
int ns_f_p(const char *p) __asm__("_ZN2ns1fEPKc");
int ns_f_p(const char *p) { return p[0]; }
int a_size(void *self) __asm__("_ZNK1A4sizeEv");
int a_size(void *self) { return self != 0; }
void a_ctor(void *self) __asm__("_ZN1AC1Ev");
void a_ctor(void *self) { (void)self; }
int other_g(void) __asm__("_ZN5other1gEv");
int other_g(void) { return 5; }
const void *a_vtable[2] __asm__("_ZTV1A") = {0, 0};
