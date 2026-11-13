/* flags: --c-std=gnu17 --target=aarch64 */
/* native-only */
/* expect-error: C3097 */
/* @TLVPPAGE / @TLVPPAGEOFF are Mach-O's thread-local descriptor references;
   on other targets they are an error (use :tlsdesc: or :gottprel:). */
extern _Thread_local long tv;
long *f(void) {
    long *p;
    __asm__("adrp %0, tv@TLVPPAGE\n\tldr %0, [%0, tv@TLVPPAGEOFF]" : "=r"(p));
    return p;
}
int main(void) { return f() == 0; }
