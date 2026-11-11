/* expect-error: C3900 */
_Thread_local int t;
int main(void) { return t; }
