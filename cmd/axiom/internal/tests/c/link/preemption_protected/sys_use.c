// Position-dependent code of the system's compiler: a copy of the object
// and the function's address without the GOT.
extern int prot_var;
int prot_fn(void);
int sys_get(void) { return prot_var; }
int (*sys_fn(void))(void) { return prot_fn; }
