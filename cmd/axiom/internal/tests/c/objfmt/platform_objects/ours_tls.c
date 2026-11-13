// This compiler's object: a thread-local and a constructor.
_Thread_local int ours_counter = 10;
int ours_ctor_ran;

int ours_tl(int add) {
    ours_counter += add;
    return ours_counter;
}

__attribute__((constructor)) static void ours_init(void) { ours_ctor_ran = 1; }
