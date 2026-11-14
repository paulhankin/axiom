/* flags: --c-std=c17 */
/* expect-error: C3005 */
/* _Complex alone is double _Complex only in a GNU dialect. */
_Complex z;
int main(void) { return 0; }
