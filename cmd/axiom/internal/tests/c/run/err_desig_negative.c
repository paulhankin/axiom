/* expect-error: C3072 */
int a[3] = { [-1] = 1 };
int main(void) { return 0; }
