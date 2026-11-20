#include <dlfcn.h>
#include <stdio.h>

int lib_api(int x);
int host_value(void) { return 10; }
int host_only(int x) { return x * 100; }

int main(int argc, char **argv) {
    printf("%d\n", lib_api(4));
    if (argc > 1) {
        void *h = dlopen(argv[1], RTLD_NOW);
        if (!h) {
            printf("dlopen: %s\n", dlerror());
            return 1;
        }
        int (*f)(int) = (int (*)(int))dlsym(h, "lib_api");
        printf("%d %s\n", f ? f(1) : -1, dlsym(h, "lib_inner") ? "inner exported" : "inner hidden");
    }
    if (argc > 2) {
        void *h = dlopen(argv[2], RTLD_NOW);
        if (!h) {
            printf("dlopen: %s\n", dlerror());
            return 1;
        }
        int (*run)(int) = (int (*)(int))dlsym(h, "plugin_run");
        printf("%d\n", run(2));
    }
    return 0;
}
