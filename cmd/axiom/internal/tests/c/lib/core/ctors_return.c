/* Constructors and destructors (GNU dialects) when main returns: the
   destructors run once, from exit, which main's return calls (5.1.2.2.3). */
/* flags: --c-std=gnu99 */
#include <stdio.h>

__attribute__((constructor)) static void before(void) { printf("before\n"); }
__attribute__((destructor)) static void after(void) { printf("after\n"); }

int main(void) {
    printf("main\n");
    return 0;
}
/* expect: before */
/* expect: main */
/* expect: after */
