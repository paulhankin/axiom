/* The builtin behind <stdarg.h>'s va_arg: an expression, then a type name
   (a typedef name, a derived type, a structure), and postfix operators on
   its result. */
typedef int T;
struct s { int m; };
int f(int n, ...) {
    __builtin_va_list ap;
    int a = __builtin_va_arg(ap, T);
    char *p = __builtin_va_arg(ap, char *);
    int (*fp)(void) = __builtin_va_arg(ap, int (*)(void));
    int m = __builtin_va_arg(ap, struct s).m;
    return a + m + __builtin_va_arg(*&ap, unsigned long) + (p != 0) + (fp != 0);
}
