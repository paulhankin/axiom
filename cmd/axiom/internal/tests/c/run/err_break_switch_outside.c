/* break inside a function but outside any loop or switch. */
/* expect-error: C3042 */
int main(void) { if (1) break; return 0; }
