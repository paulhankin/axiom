/* An enumeration constant hides a typedef name; a parameter does too. */
typedef int T;
int f(void) {
    enum { T = 3 };
    int x = T;
    return x;
}
int g(int T) {
    return T * 2;
}
T h(T x) {
    return x;
}
