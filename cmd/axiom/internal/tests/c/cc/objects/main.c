#include <math.h>
#include <stdio.h>

int twice(int x);
int thrice(int x);

int main(int argc, char **argv) {
    (void)argv;
    printf("%d %d %.0f\n", twice(20), thrice(argc), sqrt(49.0));
    return 7;
}
