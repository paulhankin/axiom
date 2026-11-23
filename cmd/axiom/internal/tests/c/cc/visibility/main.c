#include <stdio.h>
int attr_default(void);
int pushed_default(void);
int attr_over_pragma(void);
int after_pop(void);
int prot(void);
int entry(void);
extern int data_default;
/* the library's own() is hidden: its calls stay its own */
int own(void) { return 99; }
int main(void) {
    printf("%d %d %d %d %d %d %d\n", attr_default(), pushed_default(), attr_over_pragma(), after_pop(), prot(),
           data_default, entry());
    return 0;
}
