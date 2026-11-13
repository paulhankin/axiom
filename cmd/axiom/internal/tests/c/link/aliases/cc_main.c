#include <stdio.h>
extern int pub_data;
int pub_fn(void);
int pub_weak(void);
int main(void) {
    printf("%d %d %d\n", pub_data, pub_fn(), pub_weak());
    return 0;
}
