#include <stdio.h>
int ours_a(int);
int ours_b(const char *);
int ourlib_x(int);
int main(void) {
    printf("%d %d %d\n", ours_a(4), ours_b("abc"), ourlib_x(6));
    return 0;
}
