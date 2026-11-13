/* __isoc23_strtoul (the system interface of Linux programs): C23's form of the function (which takes the 0b prefix: the system's headers name it so in C23).
   An object compiled against the system C library's headers refers to it
   (docs/c.md, "Objects and libraries of other compilers"): defined here so
   that the program's C library is this one (--c-lib=axiom). The interface
   is the Linux Standard Base's (Core specification, the generic and the
   architecture parts); nothing of the system library's own layout is
   assumed beyond what that specification states. */
unsigned long __axc_strtoul_c23(const char *s, char **end, int base);

unsigned long __isoc23_strtoul(const char *s, char **end, int base) {
    return __axc_strtoul_c23(s, end, base);
}
