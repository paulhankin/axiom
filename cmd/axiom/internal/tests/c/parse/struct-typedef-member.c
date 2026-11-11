/* A struct member declaration list with typedefs and nested tags. */
typedef struct pt { int x, y; } pt;
typedef struct { pt a, b; } line;
line l = { { 1, 2 }, { 3, 4 } };
int f(line *p) { return p->a.x + p->b.y; }
