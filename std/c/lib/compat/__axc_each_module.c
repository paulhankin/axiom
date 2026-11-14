/* __axc_each_module (axc/modules.h): the running program's modules, from
   the program headers the kernel names in the auxiliary vector (AT_PHDR,
   AT_PHNUM: the executable's; ELF generic ABI, "Program Header") and, when
   the program is linked dynamically, the dynamic linker's list of the
   objects it loaded, which its debugging interface gives through the
   executable's DT_DEBUG entry (a struct r_debug, whose r_map is the first
   struct link_map: l_addr, l_name, l_ld, l_next, l_prev). A loaded object's
   program headers are found from its ELF header, at its first segment
   (l_addr when that segment's address is 0, as a shared object's is). The
   list is read as it is: a shared object that another thread loads or
   unloads meanwhile may be missed. */
#include <axc/modules.h>
#include <stddef.h>
#include <string.h>

unsigned long long __getauxval(unsigned long long);

enum { AT_PHDR = 3, AT_PHNUM = 5 };
enum { PT_LOAD = 1, PT_DYNAMIC = 2, PT_PHDR = 6 };
enum { DT_NULL = 0, DT_DEBUG = 21 };

/* the 64-bit ELF structures read (ELF-64 Object File Format) */
struct ehdr {
    unsigned char ident[16];
    uint16_t type, machine;
    uint32_t version;
    uint64_t entry, phoff, shoff;
    uint32_t flags;
    uint16_t ehsize, phentsize, phnum, shentsize, shnum, shstrndx;
};
struct phdr {
    uint32_t type, flags;
    uint64_t offset, vaddr, paddr, filesz, memsz, align;
};
struct dyn {
    int64_t tag;
    uint64_t val;
};
struct link_map {
    uintptr_t l_addr;
    const char *l_name;
    const struct dyn *l_ld;
    struct link_map *l_next, *l_prev;
};
struct r_debug {
    int r_version;
    struct link_map *r_map;
};

static int is_elf(const struct ehdr *h) {
    return memcmp(h->ident, "\177ELF", 4) == 0 && h->ident[4] == 2 && h->phentsize == sizeof(struct phdr);
}

/* the executable's load bias: from PT_PHDR (its headers' own address),
   else from the ELF header just before them in its first segment, else 0
   (a position-dependent executable's) */
static uintptr_t exe_bias(const struct phdr *ph, unsigned n) {
    for (unsigned i = 0; i < n; i++) {
        if (ph[i].type == PT_PHDR) return (uintptr_t)ph - (uintptr_t)ph[i].vaddr;
    }
    const struct ehdr *h = (const struct ehdr *)((uintptr_t)ph - sizeof(struct ehdr));
    for (unsigned i = 0; i < n; i++) {
        if (ph[i].type == PT_LOAD && ph[i].offset == 0) {
            if (is_elf(h) && h->phoff == sizeof(struct ehdr)) return (uintptr_t)h - (uintptr_t)ph[i].vaddr;
            break;
        }
    }
    return 0;
}

int __axc_each_module(int (*fn)(const struct __axc_module *, void *), void *data) {
    const struct phdr *ph = (const struct phdr *)(uintptr_t)__getauxval(AT_PHDR);
    unsigned n = (unsigned)__getauxval(AT_PHNUM);
    if (ph == NULL || n == 0) return 0;
    struct __axc_module m;
    m.bias = exe_bias(ph, n);
    m.name = "";
    m.phdr = ph;
    m.phnum = (uint16_t)n;
    m.link_map = NULL;
    /* the dynamic linker's list, from the executable's dynamic section */
    const struct dyn *exe_dyn = NULL;
    struct link_map *lm = NULL;
    for (unsigned i = 0; i < n; i++) {
        if (ph[i].type == PT_DYNAMIC) exe_dyn = (const struct dyn *)(m.bias + (uintptr_t)ph[i].vaddr);
    }
    for (const struct dyn *d = exe_dyn; d != NULL && d->tag != DT_NULL; d++) {
        if (d->tag == DT_DEBUG && d->val != 0) {
            const struct r_debug *r = (const struct r_debug *)(uintptr_t)d->val;
            lm = r->r_map;
        }
    }
    if (lm != NULL && lm->l_ld == exe_dyn) {
        m.link_map = lm; /* (the first entry is the executable's) */
        lm = lm->l_next;
    }
    int rc = fn(&m, data);
    for (; rc == 0 && lm != NULL; lm = lm->l_next) {
        if (lm->l_ld == exe_dyn) continue;
        const struct ehdr *h = (const struct ehdr *)lm->l_addr;
        if (h == NULL || !is_elf(h)) continue; /* (its first segment is not at its bias) */
        m.bias = lm->l_addr;
        m.name = lm->l_name != NULL ? lm->l_name : "";
        m.phdr = (const void *)(lm->l_addr + (uintptr_t)h->phoff);
        m.phnum = h->phnum;
        m.link_map = lm;
        rc = fn(&m, data);
    }
    return rc;
}
