/* expect-error: C3031 */
/* a constant expression must have a value its type can represent (6.6p4) */
static int bad = (int)1e30;
int main(void) { return bad; }
