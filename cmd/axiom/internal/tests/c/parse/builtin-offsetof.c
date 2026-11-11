/* The builtin behind <stddef.h>'s offsetof: a type name, then a member
   designator (an identifier, then members and subscripts). */
struct in { int y[3]; };
struct s { int a[4]; struct in in[2]; union { int u; } un; };
typedef struct s S;
unsigned long a = __builtin_offsetof(struct s, a);
unsigned long b = __builtin_offsetof(S, a[2]);
unsigned long c = __builtin_offsetof(S, in[1].y[2]);
unsigned long d = __builtin_offsetof(struct s, un.u) + __builtin_offsetof(struct in, y[1 ? 1 : 2]);
