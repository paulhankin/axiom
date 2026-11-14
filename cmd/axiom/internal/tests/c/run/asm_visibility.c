/* flags: --c-std=gnu17 */
/* native-only */
/* .hidden, .internal and .protected in file-scope asm reach the object
   (ELF's STV_HIDDEN and STV_PROTECTED, Mach-O's private extern: the suite
   loads a shared library of them in c/link/asm_visibility on Linux);
   the program's references still bind to the definitions. And .fill's
   forms: count, size up to 8 (the value's low 4 bytes, then zeros), the
   value omitted. */
#include "host.h"

__asm__(".data\n"
        ".globl vis_hidden, vis_internal, vis_protected, vis_fill\n"
        ".hidden vis_hidden\n"
        ".internal vis_internal\n"
        ".protected vis_protected\n"
        ".balign 4\n"
        "vis_hidden: .long 11\n"
        "vis_internal: .long 22\n"
        "vis_protected: .long 33\n"
        "vis_fill:\n"
        ".fill 2\n"
        ".fill 1, 2, 0x1234\n"
        ".fill 1, 8, 0x1122334455667788\n"
        ".fill 2, 1, 7\n"
        ".text\n");

extern int vis_hidden, vis_internal, vis_protected;
extern unsigned char vis_fill[];

int main(void) {
    print_i64(vis_hidden + vis_internal + vis_protected);
    long long sum = 0;
    for (int k = 0; k < 14; k++) sum = sum * 3 + vis_fill[k];
    print_i64(sum);
    return 0;
}
/* expect: 66 */
/* expect: 14017240 */
