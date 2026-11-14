// The library of the test (main.c): definitions of default visibility,
// which the program may preempt, and protected ones, which it may not.
int printf(const char *, ...);

int shared_var = 5;
__attribute__((visibility("protected"))) int prot_var = 7;

int hook(void) { return 1; }
__attribute__((visibility("protected"))) int phook(void) { return 2; }
int lib_fn(void) { return 3; }

int lib_report(void) {
    printf("lib: %d %d %d %d\n", shared_var, prot_var, hook(), phook());
    return shared_var;
}

int *lib_var_addr(void) { return &shared_var; }
int (*lib_hook_addr(void))(void) { return hook; }
int (*lib_fn_addr(void))(void) { return lib_fn; }
int *lib_prot_addr(void) { return &prot_var; }
