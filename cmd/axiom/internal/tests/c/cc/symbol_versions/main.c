#include <stdio.h>
int f(void);
int g(void);
int foo(void);
int call_foo(void);
int ns_f(int a) __asm__("_ZN2ns1fEi");
int ns_f_p(const char *p) __asm__("_ZN2ns1fEPKc");
extern int value;
int main(void) {
    printf("%d %d %d %d %d %d %d\n", f(), g(), foo(), call_foo(), ns_f(21), ns_f_p("A"), value);
    return 0;
}
