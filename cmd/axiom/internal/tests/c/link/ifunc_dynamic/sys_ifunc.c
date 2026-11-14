// Indirect functions of the system's compiler: a global one and a local
// one, their resolvers run before the program's code.
static int plus100(int x) { return x + 100; }
static int minus1(int x) { return x - 1; }
static void *resolve_plus(void) { return (void *)plus100; }
static void *resolve_minus(void) { return (void *)minus1; }

int sys_ifn(int) __attribute__((ifunc("resolve_plus")));
static int sys_local(int) __attribute__((ifunc("resolve_minus")));

int (*sys_ifn_addr(void))(int) { return sys_ifn; }
int sys_call_local(int x) { return sys_local(x) + sys_ifn(0); }
