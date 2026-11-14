// A cleanup that runs when an exception passes (-fexceptions: a
// personality routine and an LSDA).
void sys_inner(int *);

static void done(int **p) { (**p)++; }

int sys_cleanup(int *s) {
    int *g __attribute__((cleanup(done))) = s;
    sys_inner(s);
    return *g;
}
