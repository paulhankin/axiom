/* expect-error: C3008 */
/* In an ISO mode a flexible array member is the last member of a structure
   (6.7.2.1p3), not a member of a union. */
union u { int n; char c[]; };
int main(void) { return 0; }
