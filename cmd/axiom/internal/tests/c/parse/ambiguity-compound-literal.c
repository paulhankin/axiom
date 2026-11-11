/* A compound literal versus a cast; postfix operators apply to the literal. */
struct s { int a, b; };
int f(void) {
    int x = (int){1};
    int y = (int)1;
    int z = (struct s){.a = 1, .b = 2}.b;
    int *p = (int[]){1, 2, 3};
    return x + y + z + p[1];
}
