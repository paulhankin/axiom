/* expect-error: C3022 */
int main(void) { const int c = 0; int *p = &c; return *p; }
