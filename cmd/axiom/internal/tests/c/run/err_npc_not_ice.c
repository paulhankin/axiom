/* expect-error: C3022 */
/* (int)(0.0 + 0.0) is not an integer constant expression, so not a null pointer constant */
int main(void) { int *p = (int)(0.0 + 0.0); return p != 0; }
