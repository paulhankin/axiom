/* Declaration or expression statement: decided by whether the first
   identifier is a typedef name. */
typedef int T;
int f(int a, int b) {
    T *x;
    T(y);
    a * b;
    f(a, b);
    x = &y;
    return *x;
}
