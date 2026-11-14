/* flags: --c-std=gnu17 --features=+exception-handling,+bulk-memory,+mutable-globals */
/* wasm-only */
/* Module fields of file-scope asm in one unit (docs/c.md "Inline
   assembly"), on wasm32 and wasm64: a tag thrown by one asm statement and
   caught by another (try_table's catch_ref, whose exception throw_ref
   rethrows to the legacy form's catch), a mutable global, a passive data
   segment copied by memory.init, an element segment, a start function. */
#include "host.h"

#if __AXIOM_PTR_WIDTH__ == 64
#define PT "i64"
#else
#define PT "i32"
#endif

__asm__("(tag $oops (param i32 i64))\n"
        "(global $hits (mut i32) (i32.const 0))\n"
        "(global $ten i32 (i32.const 10))\n"
        "(data $text \"abc\")\n"
        "(elem $one func $seven)\n"
        "(func $seven (result i32) i32.const 7)\n"
        "(func $started (global.set $hits (global.get $ten)))\n"
        "(start $started)\n"
        "(func $copy (param $to " PT ") (param $n i32)\n"
        "  (memory.init $text (local.get $to) (i32.const 0) (local.get $n)))\n"
        "(func $fill_slot (param $slot i32)\n"
        "  (table.init $one (local.get $slot) (i32.const 0) (i32.const 1)))\n");

void copy(char *, int);
void fill_slot(int);

static void oops(int a, long long b) {
    __asm__ volatile("local.get %0 local.get %1 throw $oops" : : "r"(a), "r"(b));
}

/* a + b of the exception oops(a, b) throws, caught twice */
static long long twice_caught(int a, long long b) {
    long long r;
    __asm__ volatile("try (param) (result i64)\n"
                     "  block $h (result i32 i64 exnref)\n"
                     "    try_table (catch_ref $oops $h)\n"
                     "      local.get %1\n"
                     "      local.get %2\n"
                     "      call $oops\n"
                     "    end\n"
                     "    i64.const -1\n"
                     "    br 1\n"
                     "  end\n"
                     "  throw_ref\n"
                     "catch $oops\n"
                     "  local.set %0\n"
                     "  i64.extend_i32_s\n"
                     "  local.get %0\n"
                     "  i64.add\n"
                     "end\n"
                     "local.set %0"
                     : "=&r"(r)
                     : "r"(a), "r"(b));
    return r;
}

static int hits(void) {
    int r;
    __asm__ volatile("(global.set $hits (i32.add (global.get $hits) (i32.const 1)))\n"
                     "global.get $hits local.set %0"
                     : "=r"(r));
    return r;
}

static int zero(void) { return 0; }

int main(void) {
    print_i64(hits());
    print_i64(hits());
    print_i64(twice_caught(3, 40));
    char s[4] = "xyz";
    copy(s, 2);
    print_str(s, 3);
    int (*volatile f)(void) = zero;
    fill_slot((int)(__AXIOM_SIZE_TYPE__)f);
    print_i64(f());
    return 0;
}
/* expect: 11 */
/* expect: 12 */
/* expect: 43 */
/* expect: abz */
/* expect: 7 */
