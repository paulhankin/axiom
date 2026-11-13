// The system's toolchain's program that loads the compiler's shared library.
#include <dlfcn.h>
#include <stdio.h>

int main(int argc, char **argv) {
    void *h = dlopen(argv[1], RTLD_NOW);
    if (!h) {
        printf("dlopen: %s\n", dlerror());
        return 1;
    }
    int (*add)(int, int) = (int (*)(int, int))dlsym(h, "so_add");
    int (*tls)(int) = (int (*)(int))dlsym(h, "so_tls");
    const char *(*name)(int) = (const char *(*)(int))dlsym(h, "so_name");
    int (*hk)(int) = (int (*)(int))dlsym(h, "so_hook");
    void (*pr)(void) = (void (*)(void))dlsym(h, "so_print");
    int *counter = (int *)dlsym(h, "counter");
    int a = add(1, 2);
    int t1 = tls(1);
    int t2 = tls(2);
    printf("%d %d %d %s %d %d\n", a, t1, t2, name(2), hk(5), *counter);
    add(0, 0);
    add(0, 0);
    pr();
    return 0;
}
