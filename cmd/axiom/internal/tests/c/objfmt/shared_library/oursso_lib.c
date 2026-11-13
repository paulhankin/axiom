// The shared library: data with addresses, a function pointer, a
// thread-local (Windows: the DLL's TLS directory), a constructor.
int counter = 5;
static int calls;
#ifdef _WIN32
_Thread_local int tcount = 100;
#else
int tcount = 100;  // (a macOS dynamic library has no thread-locals of this compiler's)
#endif
const char *names[] = {"zero", "one", "two"};

static int twice(int x) { return 2 * x; }

int (*hook)(int) = twice;

int so_add(int a, int b) {
    calls++;
    return a + b + counter;
}

int so_tls(int d) {
    tcount += d;
    return tcount;
}

const char *so_name(int i) { return names[i]; }

int so_hook(int x) { return hook(x); }

__attribute__((constructor)) static void init(void) { counter = 7; }
