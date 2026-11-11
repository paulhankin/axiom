/* expect-error: C3900 */
int a[3] = { [1] = 5 };
int main(void) { return a[1]; }
