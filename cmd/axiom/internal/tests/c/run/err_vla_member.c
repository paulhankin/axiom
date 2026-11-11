/* A member cannot have a variably modified type. */
/* expect-error: C3081 */
int f(int n) { struct S { int (*p)[n]; } s; return sizeof s; }
int main(void) { return f(1); }
