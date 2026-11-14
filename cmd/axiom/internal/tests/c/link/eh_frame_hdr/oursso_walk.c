// Frames of this compiler's in its shared library (--emit=shared): the
// unwinder finds them through the library's PT_GNU_EH_FRAME; they call
// back into the program (sys_c).

// (internal: their addresses are their code's, which the unwinder gives)
static void so_inner(int *seen, int n, void (*cb)(int *)) {
    char buf[64];
    for (int k = 0; k < 64; k++) buf[k] = (char)(k + n);
    cb(seen);
    seen[3] += buf[n] - buf[0] - n + 39;
}

static void so_outer(int *seen, int n, void (*cb)(int *)) {
    so_inner(seen, n, cb);
    seen[3] += 0;
}

void so_addrs(void **out) {
    out[0] = (void *)so_inner;
    out[1] = (void *)so_outer;
}

void so_mid(int *seen, int n, void (*cb)(int *)) {
    so_outer(seen, n, cb);
    seen[3] += 0;
}
