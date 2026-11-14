// Code of the system's compiler that uses the library's object as the
// position-dependent code of an executable does (a copy relocation with
// -fno-pie, and on x86-64 also as a position-independent executable's),
// and takes the address of its function without the GOT (-fno-pie: the
// program's PLT stub is then the function's address).
extern int shared_var;
int lib_fn(void);

int sys_bump(void) { return ++shared_var; }
int *sys_var_addr(void) { return &shared_var; }
int (*sys_fn_addr(void))(void) { return lib_fn; }
