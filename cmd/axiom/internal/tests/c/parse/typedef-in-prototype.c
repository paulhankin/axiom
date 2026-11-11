/* A parameter hides a typedef name only in its prototype. */
typedef int T;
void f(int T, int x);
void g(T, T *);
T h;
