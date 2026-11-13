/* This compiler's object (--emit=obj, --function-sections): a function in
   the section ours_sec, a cold one, a jump table (which goes with its
   function's code), 128-bit division (the link-once helper) and what
   nothing refers to. */
__attribute__((section("ours_sec"))) int ours_a_sec(int x) { return x * 2; }

__attribute__((cold, noinline)) int ours_a_cold(int x) { return x - 3; }

int ours_a_jump(int x) {
    switch (x) {
    case 0: return 11;
    case 1: return 17;
    case 2: return 23;
    case 3: return 33;
    case 4: return 41;
    case 5: return 49;
    case 6: return 51;
    default: return 0;
    }
}

unsigned long long ours_a_div(unsigned __int128 a, unsigned long long b) { return (unsigned long long)(a / b); }

static const int unused_table[2048] = {3};
int ours_a_unused(int i) { return unused_table[i]; }
