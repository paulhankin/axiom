/* A typedef name hidden by an ordinary declaration in an inner scope, and
   visible again after it. */
typedef int T;
int f(void) {
    T a = 1;
    {
        int T;
        T = 2;
        a = T * a;
    }
    T b = a;
    {
        T T = b;
        b = T;
    }
    T *p = &b;
    return *p;
}
