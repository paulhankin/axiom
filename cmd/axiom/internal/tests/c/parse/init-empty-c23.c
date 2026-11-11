/* flags: --c-std=c23 */
/* Empty initializer braces (C23). */
struct s { int a; } v = {};
int a[3] = {};
int f(void) {
    int x = {};
    return x + (struct s){}.a;
}
