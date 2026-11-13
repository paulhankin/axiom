/* flags: --c-std=gnu17 --target=aarch64-macos */
/* native-only */
/* expect-error: C3097 */
/* A Mach-O (or PE) image is loaded above 4 GiB at an address only the loader
   knows, and its format has no relocation of a move-wide group: an address
   in a .word, or a move-wide group but the complete four of one address, is
   an error there. */
long gv;
__asm__(".data\nw: .word gv\n.text\n");
long f(void) {
    long r;
    asm("movz %0, #:abs_g1:gv\n\tmovk %0, #:abs_g0_nc:gv" : "=r"(r));
    return r;
}
int main(void) { return (int)f(); }
