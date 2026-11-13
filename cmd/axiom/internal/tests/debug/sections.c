// Debug information of code in several sections (built with
// --function-sections): a cold function (.text.unlikely), one in a section
// of its own (the section attribute) and main, each with an inline
// instance of twice, whose code is then in three sections.
int printf(const char *, ...);

static inline __attribute__((always_inline)) int twice(int v) {
    int w = v * 2;  // line: twice-body
    return w + 1;
}

__attribute__((cold, noinline)) int rare(int a) {
    int r = twice(a) + 3;
    printf("rare %d\n", r);
    return r;
}

__attribute__((section(".text.mine"), noinline)) int mine(int a) {
    int m = twice(a + 1);
    printf("mine %d\n", m);
    return m;
}

int main(int argc, char **argv) {
    (void)argv;
    int s = twice(argc);
    int t = rare(s) + mine(s);
    printf("%d\n", t);
    return t == 19 ? 0 : 1;
}
