/* expect-error: C3005 */
/* Under an ISO standard a complex type has a real floating type's parts
   (6.7.2p2): _Complex of an integer type is a GNU dialect's. */
_Complex int z;
int main(void) { return 0; }
