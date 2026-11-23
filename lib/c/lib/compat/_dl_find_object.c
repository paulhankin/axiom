/* _dl_find_object (the system interface of Linux programs; the dynamic
   linker's, as its manual documents it): the module of the program
   (axc/modules.h) whose segments hold an address, with the range they
   span, its entry in the dynamic linker's list (null in a static
   executable) and the address of its PT_GNU_EH_FRAME segment, the
   .eh_frame_hdr table that the platform's unwinder searches for the code's
   call frame information. Defined here so that code of the platform's
   toolchain (its unwinder) links with the compiler's C library, statically
   or not. 0, or -1 when no module holds the address. Only the members the
   manual gives every processor are written: dlfo_flags (0),
   dlfo_map_start, dlfo_map_end, dlfo_link_map, dlfo_eh_frame. */
#include <axc/modules.h>
#include <stddef.h>

unsigned long long __getauxval(unsigned long long);

enum { AT_PAGESZ = 6 };
enum { PT_LOAD = 1, PT_GNU_EH_FRAME = 0x6474e550 };

struct phdr {
    uint32_t type, flags;
    uint64_t offset, vaddr, paddr, filesz, memsz, align;
};

struct dl_find_object {
    unsigned long long dlfo_flags;
    void *dlfo_map_start;
    void *dlfo_map_end;
    void *dlfo_link_map;
    void *dlfo_eh_frame;
};

struct find {
    uintptr_t pc;
    struct dl_find_object *out;
    uintptr_t page; /* (the system's page size: the segments' mappings are of whole pages) */
};

static int look(const struct __axc_module *m, void *arg) {
    struct find *f = arg;
    const struct phdr *ph = m->phdr;
    uintptr_t lo = UINTPTR_MAX, hi = 0, eh = 0;
    int in = 0;
    for (unsigned i = 0; i < m->phnum; i++) {
        uintptr_t a = m->bias + (uintptr_t)ph[i].vaddr;
        if (ph[i].type == PT_LOAD) {
            uintptr_t e = a + (uintptr_t)ph[i].memsz;
            if (f->pc >= a && f->pc < e) in = 1;
            if (a - a % f->page < lo) lo = a - a % f->page;
            if (e > hi) hi = e;
        } else if (ph[i].type == PT_GNU_EH_FRAME) {
            eh = a;
        }
    }
    if (!in) return 0;
    if (hi % f->page != 0) hi += f->page - hi % f->page;
    f->out->dlfo_flags = 0;
    f->out->dlfo_map_start = (void *)lo;
    f->out->dlfo_map_end = (void *)hi;
    f->out->dlfo_link_map = m->link_map;
    f->out->dlfo_eh_frame = (void *)eh;
    return 1;
}

int _dl_find_object(void *pc, struct dl_find_object *result) {
    struct find f;
    f.pc = (uintptr_t)pc;
    f.out = result;
    f.page = (uintptr_t)__getauxval(AT_PAGESZ);
    if (f.page == 0) f.page = 4096;
    return __axc_each_module(look, &f) ? 0 : -1;
}
