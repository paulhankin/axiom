// This compiler's objects in the platform's format (Mach-O, COFF), linked
// by the compiler's linker and by the system's (cc_main.c).
// cc-imports: printf
// cc-expect: hi 5 hello 77 8
// cc-expect: 8 16
// expect: hi 5 hello 77 8
// expect: 8 16
// expect: 15
#include <stdio.h>

void say(void);
int get(void);
int lib_sum(int n);

int main(void) {
    say();
    printf("%d %d\n", get(), lib_sum(4));
    printf("%d\n", lib_sum(3) + 6);
    return 0;
}
