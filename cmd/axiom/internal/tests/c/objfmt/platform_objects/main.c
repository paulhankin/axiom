// The platform toolchain's objects (sys_*.c, compiled by clang) and a static
// library of them (arch_*.c), linked with this compiler's code and objects:
// weak definitions and references, thread-locals of both kinds,
// constructors, jump tables, a frame of several pages, common symbols.
// flags: --c-std=gnu17
// ours-flags: --c-std=gnu17
// expect: wf 42 wd 8 tl 3 4 wu 0 fp 42 ext 99
// expect: switch 99 -1 big 4094
// expect: ours 11 21 ctor 1
// expect: arch 7 common 0
#include <stdio.h>

int wf(void);
extern int wd;
int gettl(void);
int callwu(void);
extern int (*fp)(void);
int useext(void);
int sys_switch(int x);
int big_frame(int n);
int ours_tl(int add);
extern int ours_ctor_ran;
int arch_seven(void);
extern int common_one;

int ext = 99;

// (stronger than the system object's weak definition)
int wf(void) { return 42; }

int main(void) {
    int t1 = gettl();
    int t2 = gettl();
    printf("wf %d wd %d tl %d %d wu %d fp %d ext %d\n", wf(), wd, t1, t2, callwu(), fp(), useext());
    printf("switch %d %d big %d\n", sys_switch(3), sys_switch(9), big_frame(4095));
    int o1 = ours_tl(1);
    int o2 = ours_tl(10);
    printf("ours %d %d ctor %d\n", o1, o2, ours_ctor_ran);
    printf("arch %d common %d\n", arch_seven(), common_one);
    return 0;
}
