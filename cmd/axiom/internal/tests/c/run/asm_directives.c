/* flags: --c-std=gnu17 */
/* native-only */
/* The assembler language's directives in file-scope asm, on every object
   format (ELF on x86-64 and AArch64 Linux, Mach-O, PE): macros with
   defaults, :req and :vararg, .rept, .irp, .irpc, the .if family, .set,
   .equ, the expression operators, .uleb128, .sleb128, .fill, .space,
   .balign, .p2align, numeric local labels, .weak,, .type and .size (which
   only ELF records) (docs/c.md "Inline assembly"). The bytes they make
   are read by C. */
#include "host.h"

/* (the names are C's: on macOS the object adds the underscore) */
__asm__(".data\n"
        ".globl dir_len\n"
        ".balign 4\n"
        "dir_len: .long dir_end - dir_data\n"
        ".globl dir_data\n"
        ".type dir_data, @object\n"
        ".balign 8\n"
        "dir_data:\n"
        ".macro pair a, b=7\n"
        "  .byte \\a, \\b\n"
        ".endm\n"
        "pair 1\n"
        "pair 2, 3\n"
        ".macro sum first:req, rest:vararg\n"
        "  .byte \\first\n"
        "  .irp x, \\rest\n"
        "  .byte \\x\n"
        "  .endr\n"
        ".endm\n"
        "sum 4, 5, 6\n"
        ".rept 3\n"
        ".byte 8\n"
        ".endr\n"
        ".irpc c, 123\n"
        ".byte \\c\n"
        ".endr\n"
        ".set K, 10\n"
        ".equ L, K*3+1\n"
        ".if L > 30\n"
        ".byte 1\n"
        ".else\n"
        ".byte 0\n"
        ".endif\n"
        ".ifdef K\n"
        ".byte 2\n"
        ".endif\n"
        ".ifc abc, abc\n"
        ".byte 3\n"
        ".endif\n"
        ".if K == 9\n"
        ".byte 0\n"
        ".elseif (K == 10) && (3 < 4) || 0\n"
        ".byte 4\n"
        ".else\n"
        ".byte 0\n"
        ".endif\n"
        ".byte (L << 2) >> 1, L % 7, (L & 12) | 1, L ^ 5, ~L & 255, !0\n"
        ".byte 10 / 3, -1 & 0xff\n"
        ".uleb128 300\n"
        ".sleb128 -2\n"
        ".fill 2, 1, 9\n"
        ".space 2, 5\n"
        "1: .byte 0xaa\n"
        "2: .byte 2b - 1b\n"
        ".p2align 2\n"
        "dir_end:\n"
        ".size dir_data, dir_end - dir_data\n"
        ".weak dir_weak\n"
        ".balign 4\n"
        "dir_weak: .long 77\n");

extern int dir_len;
extern unsigned char dir_data[];
extern int dir_weak;

int main(void) {
    print_i64(dir_len);
    for (int k = 0; k < 34; k++) {
        print_i64(dir_data[k]);
    }
    print_i64(dir_weak);
    return 0;
}
/* expect: 36 */
/* expect: 1 */
/* expect: 7 */
/* expect: 2 */
/* expect: 3 */
/* expect: 4 */
/* expect: 5 */
/* expect: 6 */
/* expect: 8 */
/* expect: 8 */
/* expect: 8 */
/* expect: 1 */
/* expect: 2 */
/* expect: 3 */
/* expect: 1 */
/* expect: 2 */
/* expect: 3 */
/* expect: 4 */
/* expect: 62 */
/* expect: 3 */
/* expect: 13 */
/* expect: 26 */
/* expect: 224 */
/* expect: 1 */
/* expect: 3 */
/* expect: 255 */
/* expect: 172 */
/* expect: 2 */
/* expect: 126 */
/* expect: 9 */
/* expect: 9 */
/* expect: 5 */
/* expect: 5 */
/* expect: 170 */
/* expect: 1 */
/* expect: 77 */
