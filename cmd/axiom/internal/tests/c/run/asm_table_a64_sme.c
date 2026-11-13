/* flags: --c-std=gnu17 */
/* native-only */
/* The AArch64 assembler's SVE2.1 and SME scalar/regular forms (the ones
   that fit the SVE encoding machinery): rdsvl, addsvl, addspl (the
   streaming vector length), revd, and the clamps sclamp/uclamp/fclamp. Each
   with the bytes its encoding has (the Arm ARM's); assembled as file-scope
   asm and compared. Build-and-run under the emulator; elsewhere the same
   values are printed. */
#include "host.h"

#ifdef __AXIOM_ASM_AARCH64__
__asm__(".section .rodata.axiom_asm_table, \"a\"\n"
        ".globl axiom_asm_table\n"
        "axiom_asm_table:\n"
        ".arch_extension sme2\n"
        "rdsvl x0, #1\n"
        "rdsvl x7, #-32\n"
        "addsvl x0, x1, #-3\n"
        "addsvl sp, sp, #31\n"
        "addspl x2, x3, #5\n"
        "revd z0.q, p0/m, z1.q\n"
        "revd z31.q, p7/m, z30.q\n"
        "sclamp z0.b, z1.b, z2.b\n"
        "sclamp z5.h, z6.h, z7.h\n"
        "sclamp z0.s, z1.s, z2.s\n"
        "sclamp z10.d, z11.d, z12.d\n"
        "uclamp z0.b, z1.b, z2.b\n"
        "uclamp z31.d, z30.d, z29.d\n"
        "fclamp z0.h, z1.h, z2.h\n"
        "fclamp z3.s, z4.s, z5.s\n"
        "fclamp z6.d, z7.d, z8.d\n"
        ".globl axiom_asm_table_end\n"
        "axiom_asm_table_end:\n");
extern const unsigned char axiom_asm_table[];
extern const unsigned char axiom_asm_table_end[];

static int hexval(char c) { return c <= '9' ? c - '0' : c - 'a' + 10; }

int main(void) {
    static const char *want[] = {
        "2058bf04",
        "075cbf04",
        "a05f2104",
        "ff5b3f04",
        "a2586304",
        "20802e05",
        "df9f2e05",
        "20c00244",
        "c5c04744",
        "20c08244",
        "6ac1cc44",
        "20c40244",
        "dfc7dd44",
        "20246264",
        "8324a564",
        "e624e864",
    };
    const unsigned char *p = axiom_asm_table;
    int bad = 0, n = (int)(sizeof want / sizeof want[0]);
    for (int i = 0; i < n; i++) {
        const char *w = want[i];
        for (int k = 0; w[k] != 0; k += 2) {
            if (p >= axiom_asm_table_end || *p != hexval(w[k]) * 16 + hexval(w[k + 1])) {
                if (bad++ < 5) { print_str("mismatch at line ", 17); print_i64(i); }
            }
            p++;
        }
    }
    print_i64(bad);
    print_i64(p == axiom_asm_table_end);
    return 0;
}
#else
int main(void) { print_i64(0); print_i64(1); return 0; }
#endif
/* expect: 0 */
/* expect: 1 */
