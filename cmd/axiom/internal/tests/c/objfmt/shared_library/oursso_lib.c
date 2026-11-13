// The shared library: data with addresses, a function pointer, a
// thread-local (Windows: the DLL's TLS directory; macOS: its __thread_vars),
// a constructor.
int counter = 5;
static int calls;
_Thread_local int tcount = 100;
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
