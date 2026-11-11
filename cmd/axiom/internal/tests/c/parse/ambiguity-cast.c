/* A parenthesized type name is a cast; a parenthesized expression is not. */
typedef int T;
int f(int x, int (*g)(int)) {
    int a = (T)(x);
    int b = (g)(1);
    int c = (T)-1;
    int d = (x)-1;
    int e = (T)+x;
    int h = (x)+x;
    return a + b + c + d + e + h;
}
