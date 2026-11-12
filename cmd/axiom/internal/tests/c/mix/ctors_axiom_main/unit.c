/* the C unit of ctors_axiom_main/main.ax */
#include "../print.h"

static int value;

__attribute__((constructor)) static void init(void) {
    value = 42;
    print_cstr("c constructor");
}

__attribute__((destructor)) static void fini(void) { print_cstr("c destructor"); }

int c_value(void) { return value; }
