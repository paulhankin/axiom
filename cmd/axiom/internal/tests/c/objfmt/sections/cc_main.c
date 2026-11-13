// The system's compiler's main, linked by the system's linker with this
// compiler's objects, which each have a copy of the link-once helper.
int printf(const char *, ...);
int ours_a_sec(int x);
int ours_b_sec(int x);
int ours_a_cold(int x);
int ours_a_jump(int x);
long ours_a_atomic(long v);
long ours_b_atomic(long v);

int main(void) {
    printf("%d %d %d %d\n", ours_a_sec(4), ours_b_sec(5), ours_a_cold(10), ours_a_jump(3));
    printf("%ld %ld\n", ours_a_atomic(10), ours_b_atomic(5));
    return 0;
}
