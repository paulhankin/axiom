#include <stdio.h>
#include <stdlib.h>
#include "calc.h"
int main(int argc, char **argv) {
    int x = argc > 1 ? atoi(argv[1]) : 1;
    printf("%d\n", calc_scale(calc_add(x, 2)));
    return 0;
}
