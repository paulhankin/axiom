/* Aliases across units: a program's units (alias_a.c, alias_c.c, compiled
   with it as one module, where a reference to an alias is to its target),
   and this compiler's object (ours_d.c), whose aliases are symbols of their
   own, linked by the compiler's linker and by the system's (cc_main.c). A
   weak alias gives way to a definition of its name. */
/* flags: --c-std=gnu17 */
/* ours-flags: --c-std=gnu17 */
#include <stdio.h>

int api(int);
int wapi(int);
int wapi2(int);
int a_self(void);
extern int data_alias;
extern int pub_data;
int pub_fn(void);
int pub_weak(void);

int main(void) {
    data_alias += 2;
    printf("%d %d %d %d %d\n", api(10), wapi(20), wapi2(3), data_alias, a_self()); /* expect: 11 21 300 7 9 */
    printf("%d\n", (void *)api == (void *)wapi); /* expect: 1 */
    printf("%d %d %d\n", pub_data, pub_fn(), pub_weak()); /* expect: 41 3 3 */
    return 0;
}
/* cc-expect: 41 3 3 */
