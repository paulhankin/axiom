// A protected object and function of a shared library (main.c).
__attribute__((visibility("protected"))) int prot_var = 7;
__attribute__((visibility("protected"))) int prot_fn(void) { return prot_var; }
