/* A warning (an implicit conversion from a function pointer to void *), which -Werror makes an error and -w hides. */
void f(void) {}
void *p = f;
int main(void) { return 0; }
