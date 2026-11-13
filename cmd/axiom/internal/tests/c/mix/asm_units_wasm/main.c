/* Assembly sources and file-scope asm on WebAssembly (docs/c.md "Assembly
   sources", "Inline assembly"): functions in the text format in a .S
   (preprocessed: __AXIOM_ASM_WASM__) and a .s, imports of C's functions
   ("env"), calls between the units, a tail call (return_call), and
   try_table with catch_all and catch_all_ref around a longjmp, which
   throw_ref rethrows. */
/* uses: ../print.ax */
/* wasm-only */
/* flags: --c-std=gnu17 --features=+tail-call,+exception-handling */
#include <setjmp.h>
#include "../print.h"

int fma3(int, int, int);
int twice_via_c(int);
int count_down(int, int);
int caught(void (*f)(void));
int rethrown(void (*f)(void));
int via_s(int);

int c_helper(int x) { return x + 100; }

static jmp_buf env;
static void jump(void) { longjmp(env, 7); }
static void stay(void) {}

int main(void) {
    print_i64(fma3(2, 3, 4));
    print_i64(twice_via_c(5));
    print_i64(count_down(1000000, 0));
    print_i64(via_s(20));
    print_i64(caught(stay));
    int v = setjmp(env);
    if (v == 0) {
        print_i64(caught(jump));
        v = setjmp(env);
        if (v == 0) {
            rethrown(jump);
            print_i64(-1);
        } else {
            print_i64(v);
        }
    }
    return 0;
}
/* expect: 10 */
/* expect: 110 */
/* expect: 2000000 */
/* expect: 62 */
/* expect: 0 */
/* expect: 1 */
/* expect: 7 */
