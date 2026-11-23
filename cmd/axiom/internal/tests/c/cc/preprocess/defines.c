#define A 1
int a = A;
#undef A
#define F(x, y) ((x) + (y))
int b = F(2, 3);
