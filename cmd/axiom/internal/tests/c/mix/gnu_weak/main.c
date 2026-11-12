/* Weak definitions across units (docs/c.md, "C and Axiom in one program"):
   a weak definition gives way to another unit's that is not weak, of two
   weak ones the first unit's is kept, and #pragma weak makes one too. The
   constructors and destructors run though nothing refers to them (and what
   used and section keep is built with the program). */
/* uses: ../print.ax */
/* flags: --c-std=gnu11 */
#include "../print.h"

int overridden(void) __attribute__((weak));
int overridden(void) { return 1; }
int both_weak(void) __attribute__((weak));
int both_weak(void) { return 3; }
#pragma weak by_pragma
int by_pragma(void) { return 5; }
__attribute__((weak)) int weak_object = 7;

static void __attribute__((used)) kept(void) {}
int __attribute__((section("gnu_weak_data"))) placed = 9;
static void __attribute__((constructor)) start(void) { print_cstr("constructor"); }
static void __attribute__((destructor)) stop(void) { print_cstr("destructor"); }

int main(void) {
    print_i64(overridden());
    print_i64(both_weak());
    print_i64(by_pragma());
    print_i64(weak_object);
    return 0;
}
/* expect: constructor */
/* expect: 2 */
/* expect: 3 */
/* expect: 6 */
/* expect: 8 */
/* expect: destructor */
