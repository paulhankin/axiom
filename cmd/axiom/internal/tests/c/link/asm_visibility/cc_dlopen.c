// Loads the library (its path the argument) and looks up each name.
#include <dlfcn.h>
#include <stdio.h>

int main(int argc, char **argv) {
    void *h = dlopen(argv[1], RTLD_NOW);
    if (!h) return 1;
    int (*sum)(void) = (int (*)(void))dlsym(h, "vis_sum");
    printf("hidden %d internal %d protected %d default %d sum %d\n", dlsym(h, "vis_hidden") != 0,
           dlsym(h, "vis_internal") != 0, dlsym(h, "vis_protected") != 0, dlsym(h, "vis_default") != 0,
           sum ? sum() : -1);
    return 0;
}
