/* The visibility of a shared library's definitions: -fvisibility= (the
   default of those given none), #pragma GCC visibility over it, the
   attribute over both; declarations carry theirs. */
int api(void) { return 1; }
static int helper(void) { return 2; }
int internal_(void) { return helper(); }
__attribute__((visibility("default"))) int attr_default(void) { return 3; }
#pragma GCC visibility push(default)
int pushed_default(void) { return 4; }
#pragma GCC visibility push(hidden)
int pushed_hidden(void) { return 5; }
__attribute__((visibility("default"))) int attr_over_pragma(void) { return 6; }
/* declared hidden here, defined (without a visibility) in impl.c: the
   library's own, not exported, and its references bind to it */
extern int impl_func(void);
extern int impl_data;
#pragma GCC visibility pop
int after_pop(void) { return 7; }
#pragma GCC visibility pop
__attribute__((visibility("protected"))) int prot(void) { return 8; }
__attribute__((visibility("default"))) int data_default = 9;
int data_plain = 10;
/* hidden: a program's function of the name does not replace it here */
__attribute__((visibility("hidden"))) int own(void) { return 12; }
__attribute__((visibility("default"))) int entry(void) {
    int (*fp)(void) = impl_func;
    return api() + internal_() + pushed_hidden() + impl_func() + impl_data + fp() + own();
}
