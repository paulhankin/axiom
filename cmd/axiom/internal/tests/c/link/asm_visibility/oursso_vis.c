// Definitions of each visibility in file-scope asm, and a function that
// reads them all.
__asm__(".data\n"
        ".globl vis_hidden, vis_internal, vis_protected, vis_default\n"
        ".hidden vis_hidden\n"
        ".internal vis_internal\n"
        ".protected vis_protected\n"
        ".balign 4\n"
        "vis_hidden: .long 1\n"
        "vis_internal: .long 2\n"
        "vis_protected: .long 3\n"
        "vis_default: .long 4\n"
        ".text\n");

extern int vis_hidden, vis_internal, vis_protected, vis_default;

int vis_sum(void) {
    return vis_hidden + vis_internal + vis_protected + vis_default;
}
