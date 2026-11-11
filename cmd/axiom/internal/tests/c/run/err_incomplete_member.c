/* expect-error: C3007 */
struct S *p;
int main(void) { return p->x; }
