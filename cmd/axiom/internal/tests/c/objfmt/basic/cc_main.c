// The system's compiler's main, linked by the system's linker with this
// compiler's object and library (no C runtime: what the objects call is the
// system's C library's).
int printf(const char *, ...);
void say(void);
int get(void);
int lib_sum(int n);

int main(void) {
    say();
    printf("%d %d\n", get(), lib_sum(4));
    return 0;
}
