// Aliases across objects in the platforms' formats (the attribute alias, a
// GNU dialect): another name for a function or an object of the object
// that defines it (ours_d.c), a symbol of its own there, which another
// unit's references reach, whichever linker links them (cc_main.c: the
// system's). cc_link_helper.c, given with --link=, is compiled by this
// compiler for the target.
// ours-flags: --c-std=gnu17
// flags: --c-std=gnu17 --link=cc_link_helper.c
// cc-imports: printf
// cc-expect: 41 3 3
// expect: 41 3 3
// expect: helper 46
// expect: 10
#include <stdio.h>

extern int pub_data;
int pub_fn(void);
int pub_weak(void);
int link_helper(int x);

int main(void) {
    printf("%d %d %d\n", pub_data, pub_fn(), pub_weak());
    printf("%d\n", link_helper(5));
    return 0;
}
