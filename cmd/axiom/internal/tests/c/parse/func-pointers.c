/* Calls through function pointers and functions returning them. */
typedef int (*op)(int, int);
int add(int a, int b) { return a + b; }
op pick(int k) { return k ? add : 0; }
int (*pick2(int k))(int, int) { return add; }
int f(void) {
    return pick(1)(2, 3) + (*pick2(0))(4, 5);
}
