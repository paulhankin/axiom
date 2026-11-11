/* In a parameter, (T) with T a typedef name is a function taking a T; (x)
   is the parenthesized name x. */
typedef int T;
int f(int (T));
int g(int (x));
int h(int (*)(T));
int k(int (*p)(T q));
void m(T T);
