/* flags: --c-std=gnu17 --features=+exception-handling */
/* wasm-only */
/* The exception handling's legacy form in file-scope asm and asm
   statements (docs/c.md "Inline assembly"): try with catch_all, rethrow
   of the catch_all, delegate to an enclosing try (flat and folded), beside
   try_table; ref.null exn and ref.is_null. A longjmp is the exception. */
#include <setjmp.h>
#include <stdint.h>
#include "host.h"

static jmp_buf env;
static void jump(void) { longjmp(env, 1); }
static void stay(void) {}

/* a function pointer is an i64 on wasm64; call_indirect's index an i32 */
#if UINTPTR_MAX > 0xffffffffu
#define FP "i64"
#define IDX "(i32.wrap_i64 (local.get $f))"
#else
#define FP "i32"
#define IDX "(local.get $f)"
#endif

__asm__("(func $legacy (export \"legacy\") (param $f " FP ") (result i32)\n"
        "  try (result i32)\n"
        "    (call_indirect (param) (result) " IDX ")\n"
        "    i32.const 0\n"
        "  catch_all\n"
        "    i32.const 1\n"
        "  end)\n"
        ";; the catch_all rethrows; the outer try catches\n"
        "(func $again (param $f " FP ") (result i32)\n"
        "  (try $out (result i32)\n"
        "    (do\n"
        "      (try $in\n"
        "        (do (call_indirect (param) (result) " IDX "))\n"
        "        (catch_all (rethrow $in)))\n"
        "      (i32.const 0))\n"
        "    (catch_all (i32.const 2))))\n"
        ";; the inner try delegates to the outer\n"
        "(func $delegated (param $f " FP ") (result i32)\n"
        "  try $o (result i32)\n"
        "    try\n"
        "      (call_indirect (param) (result) " IDX ")\n"
        "    delegate $o\n"
        "    i32.const 0\n"
        "  catch_all\n"
        "    i32.const 3\n"
        "  end)\n");

int legacy(void (*f)(void));
int again(void (*f)(void));
int delegated(void (*f)(void));

static int (*volatile fns[3])(void (*)(void)) = {legacy, again, delegated};

int main(void) {
    for (int k = 0; k < 3; k++) {
        if (setjmp(env) == 0) {
            print_i64(fns[k](stay));
            print_i64(fns[k](jump));
        } else {
            print_i64(-1);
        }
    }
    int null;
    __asm__("ref.null exn ref.is_null local.set %0" : "=r"(null));
    print_i64(null);
    int folded;
    __asm__("(local.set %0 (try (result i32) (do (i32.const 5)) (catch_all (i32.const 6))))" : "=r"(folded));
    print_i64(folded);
    return 0;
}
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 2 */
/* expect: 0 */
/* expect: 3 */
/* expect: 1 */
/* expect: 5 */
