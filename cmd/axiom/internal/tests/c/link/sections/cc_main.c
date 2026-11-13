#include <stdio.h>
int ours_a_sec(int x);
int ours_b_sec(int x);
int ours_a_cold(int x);
int ours_a_jump(int x);
unsigned long long ours_a_div(unsigned __int128 a, unsigned long long b);
unsigned long long ours_b_rem(unsigned __int128 a, unsigned long long b);
extern char __start_ours_sec[], __stop_ours_sec[];
static int in_sec(void *p) { return (char *)p >= __start_ours_sec && (char *)p < __stop_ours_sec; }
int main(void) {
    unsigned __int128 big = ((unsigned __int128)1 << 64) + 6;
    printf("%d %d %d %d\n", ours_a_sec(4), ours_b_sec(5), ours_a_cold(10), ours_a_jump(3));
    printf("%llu %llu\n", ours_a_div(big, 7), ours_b_rem(big, 7));
    printf("%d %d\n", in_sec((void *)ours_a_sec), in_sec((void *)ours_b_sec));
    return 0;
}
