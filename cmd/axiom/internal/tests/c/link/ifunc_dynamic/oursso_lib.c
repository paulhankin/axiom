// Indirect functions of a shared library of this compiler: one exported
// (preemptible: the dynamic linker resolves the references to it, the
// library's own among them) and a protected one, which the library's own
// references reach through its PLT stub (an IRELATIVE relocation).
static int k7(int x) { return x + 7; }
static int k9(int x) { return x + 9; }
static int (*resolve7(void))(int) { return k7; }
static int (*resolve9(void))(int) { return k9; }
int so_ifn(int) __attribute__((ifunc("resolve7")));
__attribute__((visibility("protected"))) int so_pfn(int) __attribute__((ifunc("resolve9")));
int so_calls(int x) { return so_ifn(x) + so_pfn(x); }
int (*so_ifn_addr(void))(int) { return so_ifn; }
