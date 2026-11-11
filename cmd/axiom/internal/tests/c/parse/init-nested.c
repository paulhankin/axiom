/* Initializer lists, nested, with trailing commas. */
int a[2][2] = { { 1, 2 }, { 3, 4, }, };
struct { int x; int y[2]; } s = { 1, { 2, 3 } };
char str[] = "abc";
char str2[] = { "abc" };
