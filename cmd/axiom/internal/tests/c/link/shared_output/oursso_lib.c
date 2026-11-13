// The shared library: data with addresses, a function pointer, a
// thread-local, a constructor, a call of the system's C library.
int printf(const char *, ...);
unsigned long strlen(const char *);

int counter = 5;
static int calls;
_Thread_local int tcount = 100;
const char *names[] = {"zero", "one", "two"};
int (*hook)(int);

static int twice(int x) { return 2 * x; }

int so_add(int a, int b) {
    calls++;
    return a + b + counter;
}

int so_tls(int d) {
    tcount += d;
    return tcount;
}

const char *so_name(int i) { return names[i]; }

int so_hook(int x) {
    hook = twice;
    return hook(x) + (int)strlen(names[2]);
}

int so_calls(void) { return calls; }

__attribute__((constructor)) static void init(void) { counter = 7; }

void so_print(void) { printf("so says %d %d\n", counter, calls + 1); }
