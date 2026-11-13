/* Code in sections: functions with the attribute section (two units' in
   one section, between its __start_ and __stop_ symbols), cold functions
   (.text.unlikely), and with --function-sections and --data-sections each
   in a section of its own, which --gc-sections leaves out when nothing
   refers to it. This compiler's objects (ours_*.c) carry the link-once
   helper of 128-bit division, each object a COMDAT group of it, of which
   the link keeps one; the system's linker links them into a C program
   (cc_main.c), with its own garbage collection. */
/* flags: --c-std=gnu17 --function-sections --data-sections --gc-sections */
/* ours-flags: --c-std=gnu17 --function-sections --data-sections */
/* cc-flags: -Wl,--gc-sections */
#include <stdio.h>

int ours_a_sec(int x);
int ours_b_sec(int x);
int ours_a_cold(int x);
int ours_a_jump(int x);
unsigned long long ours_a_div(unsigned __int128 a, unsigned long long b);
unsigned long long ours_b_rem(unsigned __int128 a, unsigned long long b);

extern char __start_ours_sec[], __stop_ours_sec[];

__attribute__((section("ours_sec"))) int main_sec(int x) { return x + 100; }

static int in_sec(void *p) { return (char *)p >= __start_ours_sec && (char *)p < __stop_ours_sec; }

__attribute__((cold, noinline)) static int rare(int x) {
    printf("rare %d\n", x);
    return -x;
}

int unused_fn(int x) { return x * 31; } /* (left out: nothing refers to it) */
int unused_data[4096] = {1};

int main(void) {
    unsigned __int128 big = ((unsigned __int128)1 << 64) + 6;
    printf("%d %d %d\n", ours_a_sec(4), ours_b_sec(5), main_sec(1)); /* expect: 8 15 101 */
    printf("%d %d\n", ours_a_cold(10), ours_a_jump(3)); /* expect: 7 33 */
    printf("%llu %llu\n", ours_a_div(big, 7), ours_b_rem(big, 7)); /* expect: 2635249153387078803 1 */
    printf("%d %d %d\n", in_sec((void *)ours_a_sec), in_sec((void *)ours_b_sec), in_sec((void *)main_sec)); /* expect: 1 1 1 */
    printf("%d\n", rare(main_sec(-99))); /* expect: rare 1 */
    /* expect: -1 */
    return 0;
}
/* cc-expect: 8 15 7 33 */
/* cc-expect: 2635249153387078803 1 */
/* cc-expect: 1 1 */
