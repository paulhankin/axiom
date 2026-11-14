// The visibility directives of assembly reach the object (oursso_vis.c, a
// shared library of this compiler's): a hidden or internal definition is
// not exported, a protected one and one of default visibility are, which
// the system's dynamic linker shows (cc_dlopen.c: dlsym); the library's
// own code reaches all four.
// ours-flags: --c-std=gnu17
// dl-expect: hidden 0 internal 0 protected 1 default 1 sum 10
// expect: 10
int printf(const char *, ...);
int vis_sum(void);

int main(void) {
    printf("%d\n", vis_sum());
    return 0;
}
