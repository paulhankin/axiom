/* This compiler's object (--emit=obj, --function-sections): a function in
   the section ours_sec, a cold one, a jump table (which goes with its
   function's code), atomics of 32 bytes (the link-once lock helpers) and what
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

struct big { long a[4]; };
static _Atomic struct big ga; /* (atomics of 32 bytes: the link-once lock helpers) */
long ours_a_atomic(long v) {
    struct big b = {{v, v + 1, v + 2, v + 3}};
    ga = b;
    struct big c = ga;
    return c.a[2];
}

static const int unused_table[2048] = {3};
int ours_a_unused(int i) { return unused_table[i]; }
