/* This compiler's object: another function in ours_sec, and atomics of 32
   bytes (the same link-once lock helpers as ours_a's). */
__attribute__((section("ours_sec"))) int ours_b_sec(int x) { return x * 3; }

struct big { long a[4]; };
static _Atomic struct big gb; /* (the same link-once lock helpers as ours_a's) */
long ours_b_atomic(long v) {
    struct big b = {{v, v * 2, v * 3, v * 4}};
    gb = b;
    struct big c = gb;
    return c.a[3];
}
