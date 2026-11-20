/* Under -std=cNN the extensions in the reserved namespace stay (a
   conventional cc's -std=cNN): the attributes act, with the system's
   headers included first (which define __attribute__ away for a compiler
   they take for a strict one). The other names are the program's. */
#include <stdio.h>
struct __attribute__((packed)) P { char c; int i; };
struct A { char c; } __attribute__((aligned(64)));
static int ran;
__attribute__((constructor)) static void init(void) { ran = 1; }
int asm = 1, typeof = 2;  /* not keywords under an ISO standard before C23 */
int main(void) {
    __typeof__(ran) r = ran;
    printf("%zu %zu %d %d\n", sizeof(struct P), sizeof(struct A), r, asm + typeof);
#ifdef __attribute__
    puts("__attribute__ is a macro");
#endif
#if defined(linux) || defined(unix) || !defined(__STRICT_ANSI__)
    puts("not ISO's names");
#endif
    return 0;
}
