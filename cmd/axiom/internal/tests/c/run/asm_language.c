/* flags: --c-std=gnu17 */
/* native-only */
/* The assembler language of inline and file-scope asm: .macro with arguments and
   defaults, .rept and .irp, .if/.else/.ifdef conditionals, the GNU expression
   precedence, and the data directives (.octa, .uleb128, .sleb128, .ascii, .float,
   .double). The assembled bytes are compared against a table checked against the
   system assembler in development. */
#include "host.h"

#ifdef __AXIOM_ASM_X86_64__
__asm__(".section .rodata.axiom_asm_feat, \"a\"\n"
        ".globl axiom_asm_feat\n"
        "axiom_asm_feat:\n"
        ".macro pair a, b=7\n.byte \\a, \\b\n.endm\n.set k, 0\n.rept 4\n.byte k*k\n.set k, k+1\n.endr\n.irp n, 10, 20, 30\n.byte \\n\n.endr\npair 1\npair 2, 3\n.if 1+1 == 2\n.byte 0xaa\n.else\n.byte 0xbb\n.endif\n.ifdef k\n.byte 0xcc\n.endif\n.byte 1+2*3, (1+2)*3, 7&3, 7|8, 5^1, 1<<4, 255>>4, -8/2, 10%3\n.short 0x1234, -1\n.long 0x11223344\n.quad 0x1122334455667788\n.octa 0x0102030405060708090a0b0c0d0e0f10\n.uleb128 624485, 0, 127, 128\n.sleb128 -123456, 0, 63, 64\n.ascii \"hi\"\n.asciz \"yo\"\n.float 1.5, 0.0, -2.5\n.double 0.1, 1e10\n"
        ".globl axiom_asm_feat_end\n"
        "axiom_asm_feat_end:\n"
        ".text\n");

extern const unsigned char axiom_asm_feat[], axiom_asm_feat_end[];
static const char want[] = "000104090a141e01070203aacc0709030f04100ffc013412ffff443322118877665544332211100f0e0d0c0b0a090807060504030201e58e26007f8001c0bb78003fc0006869796f000000c03f00000000000020c09a9999999999b93f000000205fa00242";

static int hv(char c){return c<='9'?c-'0':c-'a'+10;}

int main(void){
    const unsigned char *p = axiom_asm_feat; int bad = 0, i = 0;
    for (; want[i]; i += 2) {
        if (p >= axiom_asm_feat_end || *p != hv(want[i])*16 + hv(want[i+1])) bad++;
        p++;
    }
    print_i64(bad); print_i64(p == axiom_asm_feat_end); return 0;
}
#else
int main(void){print_i64(0);print_i64(1);return 0;}
#endif
/* expect: 0 */
/* expect: 1 */
