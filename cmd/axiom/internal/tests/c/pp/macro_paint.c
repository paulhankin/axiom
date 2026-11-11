/* A macro is not replaced in its own replacement ("painted" names stay so). */
#define f(x) g
#define g f
f(1)(2)
#define h(x) x+f
h(1)(2)
#define F(a) a*G
#define G(a) F(a)
F(2)(9)
#define AA BB
#define BB AA
AA BB
#define self self + 1
self
#define cyc1(x) cyc2(x)
#define cyc2(x) cyc1(x)
cyc1(5)
#define id(x) x
id(id)(id)(3)
#define lp id(
lp 4)
#define AP(x) x AP
AP(AP)(1)(2)
#define NIL(x) x
#define G0 NIL(G0)
G0
