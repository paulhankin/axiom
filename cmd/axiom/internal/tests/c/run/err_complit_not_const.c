/* expect-error: C3031 */
int x = 1;
int *p = &(int){ x };
int main(void) { return 0; }
