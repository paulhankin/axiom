/* expect-error: C3035 */
int k = 1;
int a[3] = { [k] = 1 };
int main(void) { return 0; }
