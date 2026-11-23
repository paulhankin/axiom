#include <stdio.h>
double sin(double);
int main(int argc, char **argv) {
    (void)argv;
    printf("%.3f\n", sin(argc - 1));
    return 0;
}
