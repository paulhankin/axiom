/* expect-error: C1015 */
/* Under an ISO standard an integer constant takes no imaginary suffix. */
int main(void) { return (int)sizeof(2i); }
