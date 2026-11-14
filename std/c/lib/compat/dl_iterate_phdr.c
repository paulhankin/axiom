/* dl_iterate_phdr (the system interface of Linux programs): calls a
   function with the program headers of each of the program's modules, the
   executable and the shared objects loaded with it (axc/modules.h). Code
   of the platform's toolchain calls it to find the call frame information
   of the code it unwinds (docs/c.md, "Objects and libraries"): defined here
   so that such code links with the compiler's C library, statically or
   not. The interface is the Linux Standard Base's (Core specification):
   the structure has dlpi_addr, dlpi_name, dlpi_phdr and dlpi_phnum, and the
   size given the function is that of those four members, so that it reads
   no others (the counts of loaded and unloaded objects are not kept:
   whoever would cache by them sees a size without them). */
#include <axc/modules.h>
#include <stddef.h>

struct dl_phdr_info {
    uintptr_t dlpi_addr;
    const char *dlpi_name;
    const void *dlpi_phdr;
    uint16_t dlpi_phnum;
};

struct call {
    int (*fn)(struct dl_phdr_info *, size_t, void *);
    void *data;
};

static int one(const struct __axc_module *m, void *arg) {
    struct call *c = arg;
    struct dl_phdr_info info;
    info.dlpi_addr = m->bias;
    info.dlpi_name = m->name;
    info.dlpi_phdr = m->phdr;
    info.dlpi_phnum = m->phnum;
    return c->fn(&info, sizeof info, c->data);
}

int dl_iterate_phdr(int (*fn)(struct dl_phdr_info *, size_t, void *), void *data) {
    struct call c;
    c.fn = fn;
    c.data = data;
    return __axc_each_module(one, &c);
}
