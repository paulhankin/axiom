// A protected definition of a shared library binds the library's own
// references to itself: position-dependent code of a program cannot have
// a copy of the object (the library would keep using its own) nor make its
// PLT stub the function's address (the library's would differ), so such a
// reference is an error.
// flags: --c-lib=system
// ours-flags: --c-std=gnu17
// cc-variants: -O2 -fno-pie
// expect-error: E9004
// expect-stderr: but it is protected in liboursso.so
int sys_get(void);
int main(void) { return sys_get() != 7; }
