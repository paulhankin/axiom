// Code in sections, in the platforms' formats: functions with the
// attribute section, a cold one, a jump table, each function in a section
// of its own (--function-sections), and in both objects the link-once lock
// helpers of atomics wider than the processor's: COMDAT sections in COFF
// (their jump tables, frame tables and unwind information associative to
// them) and weak definitions in Mach-O, of which a link keeps one; linked
// by the compiler's linker (with --gc-sections) and by the system's
// (cc_main.c).
// flags: --c-std=gnu17 --function-sections --data-sections --gc-sections
// ours-flags: --c-std=gnu17 --function-sections --data-sections
// cc-imports: printf
// cc-expect: 8 15 7 33
// cc-expect: 12 20
// expect: 8 15 7 33
// expect: 12 20
// expect: 101
#include <stdio.h>

int ours_a_sec(int x);
int ours_b_sec(int x);
int ours_a_cold(int x);
int ours_a_jump(int x);
long ours_a_atomic(long v);
long ours_b_atomic(long v);

__attribute__((section("ours_sec"))) int main_sec(int x) { return x + 100; }

int unused_fn(int x) { return x * 31; }

int main(void) {
    printf("%d %d %d %d\n", ours_a_sec(4), ours_b_sec(5), ours_a_cold(10), ours_a_jump(3));
    printf("%ld %ld\n", ours_a_atomic(10), ours_b_atomic(5));
    printf("%d\n", main_sec(1));
    return 0;
}
