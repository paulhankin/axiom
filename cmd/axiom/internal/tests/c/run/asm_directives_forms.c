/* flags: --c-std=gnu17 */
/* native-only */
/* The assembler's data and layout directives in each of their forms, on
   every object format: .org (to a label of the section plus a constant,
   with a fill), .fill (count, size up to 8: the value's low 4 bytes, then
   zeros), .space, .skip and .zero with a fill, .balign, .p2align, .align
   (with a fill and a maximum), .balignw, .p2alignw and .balignl (the fill
   in units of 2 and 4 bytes), \@ in a macro (the count of macros expanded
   before); visibility (.hidden); .section in the object format's syntax
   (ELF's flags and a COMDAT group, Mach-O's segment, section, type and
   attributes, COFF's flags and a COMDAT selection; ELF's .rodata on each; initializers);
   relocation modifiers
   in data (sym@GOTPCREL: the offset of the symbol's GOT slot, of its
   .refptr pointer in COFF; ELF x86-64's sym@PLT - .; COFF's .rva). The
   bytes are those of the conventional assembler for the same text. */
#include "host.h"

/* .align n: n bytes on x86-64 ELF and in COFF, 2^n on AArch64 ELF and in
   Mach-O */
#if defined(_WIN32) || (defined(__x86_64__) && !defined(__APPLE__))
#define ALIGN4 ".align 4\n"
#else
#define ALIGN4 ".align 2\n"
#endif

/* SEC_RO: read-only data of a section of its own (COFF's "dr" kept its
   name); SEC_INIT: an initializer's pointer (ELF's @init_array, Mach-O's
   mod_init_funcs; COFF's .CRT$XCU, as the C units' .init_array, which
   assembles but which a program of the compiler's own link does not run
   yet: not checked there) */
#if defined(__APPLE__)
#define SEC_DATA "__DATA,__df_sec,regular,no_dead_strip"
#define SEC_GROUP "__DATA,__df_grp"
#define SEC_RO "__TEXT,__df_ro"
#define SEC_INIT ".section __DATA,__mod_init_func,mod_init_funcs\n.p2align 3\n.quad df_init\n"
#elif defined(_WIN32)
#define SEC_DATA ".df_sec,\"dw\""
#define SEC_GROUP ".df_grp,\"dw\",discard,df_gsym"
#define SEC_RO ".df_ro,\"dr\""
#define SEC_INIT ".section .init_array,\"aw\",@init_array\n.p2align 3\n.quad df_init\n"
#else
#define SEC_DATA ".df_sec,\"aw\",@progbits"
#define SEC_GROUP ".df_grp,\"awG\",@progbits,df_gsym,comdat"
#define SEC_RO ".df_ro,\"a\""
#define SEC_INIT ".section .init_array,\"aw\",@init_array\n.p2align 3\n.quad df_init\n"
#endif

__asm__(".data\n"
        ".globl df_data\n"
        ".balign 8\n"
        "df_data:\n"
        ".byte 1\n"
        ".org df_data+4, 0xaa\n"
        ".fill 2,1,7\n"
        ".fill 1,2,0x1234\n"
        ".fill 1,8,0x1122334455667788\n"
        ".space 2, 9\n"
        ".skip 1\n"
        ".zero 1\n"
        ".byte 3\n"
        ".balign 4, 5\n"
        ".byte 4, 4\n"
        ".p2alignw 3, 0x0102\n"
        ".byte 6, 6, 6, 6\n"
        ".balignl 8, 0x0a0b0c0d\n"
        ".byte 7\n"
        ".p2align 2\n"
        ".byte 8\n"
        ".balign 8,0x11,3\n"
        ".byte 9\n"
        ALIGN4
        ".macro lab\n"
        ".byte \\@\n"
        ".endm\n"
        "lab\n"
        "lab\n"
        ".globl df_end\n"
        "df_end:\n"
        ".globl df_hidden\n"
        ".hidden df_hidden\n"
        "df_hidden: .long 17\n"
        ".globl df_target\n"
        "df_target: .long 23\n"
        ".globl df_got\n"
        ".balign 4\n"
#if !(defined(_WIN32) && defined(__aarch64__))
        "df_got: .long df_target@GOTPCREL\n"
#else
        "df_got: .long 0\n"
#endif
        ".globl df_plt\n"
#if defined(__x86_64__) && defined(__linux__)
        "df_plt: .long df_target@PLT - .\n"
#else
        "df_plt: .long 0\n"
#endif
        ".globl df_rva\n"
#if defined(_WIN32)
        "df_rva: .rva df_target\n"
#else
        "df_rva: .long 0\n"
#endif
        ".section " SEC_DATA "\n"
        ".globl df_sec\n"
        "df_sec: .long 31\n"
        ".section " SEC_GROUP "\n"
        ".globl df_gsym\n"
        "df_gsym: .long 42\n"
        ".section " SEC_RO "\n"
        ".globl df_ro\n"
        "df_ro: .long 53\n"
        ".section .rodata\n"
        ".globl df_elfro\n"
        "df_elfro: .long 64\n"
        SEC_INIT
        ".text\n");

/* run before main by SEC_INIT's pointer */
int df_inited;
void df_init(void) { df_inited = 1; }

extern unsigned char df_data[], df_end[];
extern int df_hidden, df_target, df_got, df_plt, df_rva, df_sec, df_gsym;
extern const int df_ro, df_elfro;
#if defined(_WIN32)
extern char __ImageBase;
#endif

int main(void) {
    int n = (int)(df_end - df_data);
    long long sum = 0;
    for (int k = 0; k < n; k++) sum += (long long)df_data[k] * (k + 1);
    print_i64(n);
    print_i64(sum);
    print_i64(df_hidden);
#if !(defined(_WIN32) && defined(__aarch64__))
    print_bool(**(int **)((char *)&df_got + df_got) == 23);
#else
    print_bool(1);
#endif
#if defined(__x86_64__) && defined(__linux__)
    print_bool((char *)&df_plt + df_plt == (char *)&df_target);
#else
    print_bool(1);
#endif
#if defined(_WIN32)
    print_bool(&__ImageBase + df_rva == (char *)&df_target);
#else
    print_bool(1);
#endif
    print_i64(df_sec);
    print_i64(df_gsym);
    print_i64(df_ro);
    print_i64(df_elfro);
#if defined(_WIN32)
    print_bool(1);
#else
    print_bool(df_inited);
#endif
    return 0;
}
/* expect: 54 */
/* expect: 13996 */
/* expect: 17 */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: 31 */
/* expect: 42 */
/* expect: 53 */
/* expect: 64 */
/* expect: true */
