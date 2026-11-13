/* This compiler's object: another function in ours_sec, and 128-bit
   remainders (the same link-once helper as ours_a's). */
__attribute__((section("ours_sec"))) int ours_b_sec(int x) { return x * 3; }

unsigned long long ours_b_rem(unsigned __int128 a, unsigned long long b) { return (unsigned long long)(a % b); }
