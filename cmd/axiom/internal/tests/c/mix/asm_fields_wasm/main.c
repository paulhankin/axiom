/* Module fields of an assembly source and of file-scope asm on WebAssembly
   (docs/c.md "Inline assembly"): a tag defined in the .S that the C unit's
   asm statements throw and catch by name (try_table's catch and the legacy
   catch), a mutable global both units read and write, an immutable global
   of the C unit's that initializes one of the .S, globals of every type,
   memory.init and data.drop of a passive data segment, table.init and
   elem.drop of an element segment, table.copy, a start function, and
   exports of a global and a tag. */
/* uses: ../print.ax */
/* wasm-only */
/* flags: --c-std=gnu17 --features=+exception-handling,+bulk-memory,+mutable-globals,+simd128 */
#include "../print.h"

__asm__("(import \"env\" \"err\" (tag $err (param i32)))\n"
        "(import \"env\" \"counter\" (global $counter (mut i32)))\n"
        "(global $base (export \"base\") i32 (i32.const 100))\n");

int bump(int);
int derived_value(void);
long long big_value(void);
double half_value(void);
int lane2(void);
int exn_is_null(void);
int copy_msg(char *);
void patch(int);
void copy_slot(int);
int catch_err(void (*)(int));

/* throws $err with v */
void raise_err(int v) {
    __asm__ volatile("local.get %0 throw $err" : : "r"(v));
}

/* v thrown and caught here: try_table (catch $err) */
static int caught_new(int v) {
    int r;
    __asm__ volatile("block $done\n"
                     "  block $h (result i32)\n"
                     "    try_table (catch $err $h)\n"
                     "      local.get %1\n"
                     "      call $raise_err\n"
                     "    end\n"
                     "    i32.const -1\n"
                     "    local.set %0\n"
                     "    br $done\n"
                     "  end\n"
                     "  local.set %0\n"
                     "end"
                     : "=r"(r)
                     : "r"(v));
    return r;
}

/* ... and by the legacy form's catch $err */
static int caught_old(int v) {
    int r;
    __asm__ volatile("try (result i32)\n"
                     "  local.get %1\n"
                     "  call $raise_err\n"
                     "  i32.const -1\n"
                     "catch $err\n"
                     "  i32.const 1\n"
                     "  i32.add\n"
                     "catch_all\n"
                     "  i32.const -2\n"
                     "end\n"
                     "local.set %0"
                     : "=r"(r)
                     : "r"(v));
    return r;
}

static int counter(void) {
    int r;
    __asm__ volatile("global.get $counter local.set %0" : "=r"(r));
    return r;
}

static void set_counter(int v) {
    __asm__ volatile("local.get %0 global.set $counter" : : "r"(v));
}

static int ten(void) { return 10; }

int main(void) {
    print_i64(counter());
    print_i64(bump(3));
    set_counter(40);
    print_i64(bump(2));
    print_i64(counter());
    print_i64(derived_value());
    print_i64(big_value());
    print_f64(half_value());
    print_i64(lane2());
    print_i64(exn_is_null());
    char buf[8] = "xxxxxxx";
    print_i64(copy_msg(buf));
    print_cstr(buf);
    print_i64(caught_new(5));
    print_i64(caught_old(6));
    print_i64(catch_err(raise_err));
    int (*volatile p)(void) = ten;
    print_i64(p());
    copy_slot((int)(__AXIOM_SIZE_TYPE__)p);
    print_i64(p());
    patch((int)(__AXIOM_SIZE_TYPE__)p);
    print_i64(p());
    return 0;
}
/* expect: 15 */
/* expect: 18 */
/* expect: 42 */
/* expect: 42 */
/* expect: 100 */
/* expect: -1234567890123 */
/* expect: 3 */
/* expect: 3 */
/* expect: 1 */
/* expect: 6 */
/* expect: hello */
/* expect: 5 */
/* expect: 7 */
/* expect: 1003 */
/* expect: 10 */
/* expect: 10 */
/* expect: 2 */
