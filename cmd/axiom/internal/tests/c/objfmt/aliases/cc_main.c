// The system's compiler's main, linked by the system's linker with this
// compiler's object, whose aliases it calls and reads.
int printf(const char *, ...);
extern int pub_data;
int pub_fn(void);
int pub_weak(void);

int main(void) {
    printf("%d %d %d\n", pub_data, pub_fn(), pub_weak());
    return 0;
}
