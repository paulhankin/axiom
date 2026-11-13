/* flags: --c-std=gnu17 --target=aarch64 */
/* native-only */
/* expect-error: C3096 */
/* expect-error: C3901 */
/* AArch64 constraints that do not fit their operands: an object too wide for
   an SVE predicate (32 bytes) or vector (256 bytes) register, more SVE
   operands than registers, a constraint the dialect does not have, and an
   asm goto with an output that goes through memory after the template. */
struct p33 { unsigned char b[33]; };
struct z257 { unsigned char b[257]; };
struct z32 { unsigned char b[32]; };
int main(void) {
    struct p33 p;
    struct z257 z;
    struct z32 a, b, c, d, e, f, g, h, i;
    asm("" : "=Upa"(p));
    asm("" : "=w"(z));
    asm("" : "=y"(a), "=y"(b), "=y"(c), "=y"(d), "=y"(e), "=y"(f), "=y"(g), "=y"(h), "=y"(i));
    asm("" : : "Uqq"(0));
    asm goto("" : "=w"(a) : : : out);
out:
    return 0;
}
