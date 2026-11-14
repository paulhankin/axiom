/* axc/modules.h - the modules of the running program (the executable and,
   when it is linked dynamically, the shared objects loaded with it), for
   dl_iterate_phdr and _dl_find_object (std/c/lib/compat/), through which
   code of the platform's toolchain (its unwinder) finds the program
   headers and call frame tables of the code it walks. Private to the
   compiler's library. */
#ifndef __AXIOM_AXC_MODULES_H
#define __AXIOM_AXC_MODULES_H

#include <stdint.h>

/* a module: its load bias (what its program headers' addresses are
   offset by), its file's name ("" for the executable), its program
   headers (64-bit ELF ones) and their number, and its entry in the
   dynamic linker's list of loaded objects (0 for a static executable) */
struct __axc_module {
    uintptr_t bias;
    const char *name;
    const void *phdr;
    uint16_t phnum;
    void *link_map;
};

/* calls fn with each module, the executable first, until it returns
   nonzero (which is the result; 0 when every call returned 0) */
int __axc_each_module(int (*__fn)(const struct __axc_module *, void *), void *__data);

#endif
