// Symbol preemption between a program and a shared library of this
// compiler (--emit=shared): the library's definitions of default
// visibility are preemptible, its references to them bound through its
// GOT and PLT by the dynamic linker, so that the program's definition of
// hook replaces the library's for the library too, and the program's copy
// of shared_var (a copy relocation, which code of the system's compiler
// asks for) is the one object; protected definitions (phook, prot_var)
// bind within the library. A function's address is one address in every
// module, the program's PLT stub when its position-dependent code takes it
// (canonical).
// flags: --c-lib=system
// ours-flags: --c-std=gnu17
// cc-variants: -O2 | -O2 -fno-pie
// expect: lib: 6 7 10 2
// expect: 6 1 1 1 7 1
int printf(const char *, ...);

int hook(void) { return 10; }   // preempts the library's
int phook(void) { return 20; }  // not the library's: it is protected there

int lib_report(void);
int *lib_var_addr(void);
int (*lib_hook_addr(void))(void);
int (*lib_fn_addr(void))(void);
int *lib_prot_addr(void);
int lib_fn(void);
extern int prot_var;
int sys_bump(void);
int *sys_var_addr(void);
int (*sys_fn_addr(void))(void);

int main(void) {
    sys_bump();
    int v = lib_report();
    printf("%d %d %d %d %d %d\n", v, lib_var_addr() == sys_var_addr(), lib_hook_addr() == hook,
           lib_fn_addr() == sys_fn_addr() && sys_fn_addr() == lib_fn, prot_var + phook() - 20,
           lib_prot_addr() == &prot_var);
    return 0;
}
