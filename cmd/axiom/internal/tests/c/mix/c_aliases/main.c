/* Aliases across a program's units (a GNU dialect): another unit's
   declaration of an alias's name is its target (one module: ir.Alias), on
   every target; a weak alias gives way to another unit's definition. */
/* flags: --c-std=gnu17 */
#include <stdio.h>

int api(int);
int wapi(int);
int wapi2(int);
int a_self(void);
extern int data_alias;

int main(void) {
    data_alias += 2;
    printf("%d %d %d %d %d\n", api(10), wapi(20), wapi2(3), data_alias, a_self()); /* expect: 11 21 300 7 9 */
    printf("%d\n", (void *)api == (void *)wapi); /* expect: 1 */
    return 0;
}
