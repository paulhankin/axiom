/* The variables of a C program in its debug information (run.mjs, "debug
   information"; dwarf.mjs reads it): block scopes (lexical blocks, a name
   declared again inside), typedefs, a block-scope static, structure and
   typedef declared in a block, a variable length array (its bound),
   a thread-local object, and values a debugger reads at -O1 and above
   (a parameter, a variable assigned once, one whose address is taken; one
   assigned again and again, a structure taken apart, a pointer into a
   frame object, a variable length array). */
typedef int count_t;
typedef count_t *count_ptr;
_Thread_local int per_thread = 3;
static volatile int sink;

static void use(int *p) { sink = *p; }

int shadow(count_t n) {
    count_t x = n + 1;
    count_ptr px = &x;
    for (int i = 0; i < n; i++) {
        int x = i * 2;                           /* line: inner-x */
        typedef long wide_t;
        struct pair { int a, b; } pr = { x, i };
        static int calls;
        wide_t w = x + calls++;
        sink = (int)w + pr.a + pr.b;
    }
    {
        int x = 99;
        sink = x;
    }
    return *px;
}

int vla(int n) {
    int v[n];
    for (int i = 0; i < n; i++) {
        v[i] = i * i;
    }
    sink = v[n - 1];                             /* line: vla-use */
    return v[0];
}

int once(int a, int b) {
    int s = a * b + per_thread;
    int t = s - a;
    use(&t);
    sink = s;                                    /* line: once-use */
    return s + t;
}

struct pt { int x; long y; };
int *volatile keep;

int again(int a, int b) {
    int r = a + 1;
    sink = r;
    r = r * b;
    sink = r;
    struct pt q = { a, b };
    q.x += 3;
    sink = q.x;
    int arr[4] = { a, b, 3, 4 };
    keep = arr;                                  /* (in memory: its address escapes) */
    int *pa = &arr[2];
    sink = *pa;
    for (int i = 0; i < a; i++) {
        r = r ^ i;
        q.x += i;
        sink = r + q.x;
    }
    return (int)(r + q.x + q.y) + arr[1];
}

int main(int argc, char **argv) {
    (void)argv;
    per_thread++;
    return shadow(argc + 2) + vla(argc + 3) + once(argc, 5) + again(argc, 7) == 0;
}
