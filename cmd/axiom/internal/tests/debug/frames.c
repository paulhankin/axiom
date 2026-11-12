/* The debug information of a C program (run.mjs, "debug information";
   dwarf.mjs checks it): functions whose frames differ - a leaf without a
   frame, one with many callee-saved registers, a variable length array (the
   stack pointer moves), an early exit before the frame is set up
   (shrink-wrapping at -O2), a call in tail position (a sibling call at -O2),
   a function under #line. The lines named in run.mjs are marked. */
static volatile int sink;

static int leaf(int x) { return x * 3 + 1; }

static int many(int a, int b, int c, int d) {
    int s = 0;
    for (int i = 0; i < a; i++) {
        s += leaf(i) * b + c * d;            /* line: many-loop */
        sink = s;
    }
    return s + a * b * c * d;
}

static int vla(int n) {
    int v[n];                                /* line: vla-decl */
    for (int i = 0; i < n; i++) {
        v[i] = leaf(i);
    }
    int t = 0;
    for (int i = 0; i < n; i++) {
        t += v[i];
    }
    return t;
}

static int early(int *p) {
    if (!p) {
        return -1;
    }
    sink = many(*p, 2, 3, 4);
    return sink;
}

static int tail(int x) {
    if (x > 100) {
        return x;
    }
    return vla(x % 7 + 1);                   /* line: tail-call */
}

#line 500 "generated.y"
static int presumed(int x) {
    return x + 1;
}
#line 52 "frames.c"
int add(int a, int b) {
    int s = a + b;                           /* line: add-sum */
    return early(&s) + tail(s) + presumed(s);
}

int main(int argc, char **argv) {
    (void)argv;
    return add(argc + 1, 3) == 0;            /* (argc: nothing folds away) */
}
