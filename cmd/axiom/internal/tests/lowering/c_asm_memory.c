/* An asm statement's memory operands without a "memory" clobber: it reads its
   input objects and writes its output ones, and nothing else (the dialect's
   rule). So at -O2 another object's value stays in a register across it (fwd:
   one load of `other`; with the clobber, fwd_clobber, two), a store to the
   named object before it is kept (keep_store: #555), a load of it after it is
   done again (reload), and a store to another object that a later store
   overwrites is dead (dead_other: no #888). Four 32-bit loads in all. */
// lower: --target=aarch64 -O2 --c-freestanding --c-std=gnu17 | \bldr\s+w | 4
// lower: --target=aarch64 -O2 --c-freestanding --c-std=gnu17 | #0x22b\b | 1
// lower: --target=aarch64 -O2 --c-freestanding --c-std=gnu17 | #0x378\b | 0
// lower: --target=aarch64 -O2 --c-freestanding --c-std=gnu17 | #0x3e7\b | 1
int named, other, clobbered;
int fwd(void) {
    int a = other;
    __asm__("" : "=m"(named));
    return a + other;
}
int fwd_clobber(void) {
    int a = clobbered;
    __asm__("" : "=m"(named) : : "memory");
    return a + clobbered;
}
void keep_store(void) {
    named = 555;
    __asm__("" : : "m"(named));
    named = 666;
}
int reload(void) {
    named = 777;
    __asm__("" : "+m"(named));
    return named;
}
void dead_other(void) {
    other = 888;
    __asm__("" : : "m"(named));
    other = 999;
}
