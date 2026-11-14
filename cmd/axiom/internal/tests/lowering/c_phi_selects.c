/* Accumulators updated by selects in a loop (bad, nf) are built in their
   phis' registers (x64.ax acc_select: a copy of the old value, then a cmov
   of the new one, here cmove), and the flag `prev = f` is computed after the
   old flag's last use (sink_phi_inputs): the loop that runs once per
   element copies nothing at its back edge. */
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bcmove\s | 4
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bmov\s+%r\w+,%r\w+\n | 5
unsigned long walk(const unsigned long *a, unsigned long n, int *nfree) {
    int bad = 0, nf = 0;
    _Bool prev = 0;
    for (unsigned long i = 0; i < n; i++) {
        unsigned long w = a[i];
        if (w != a[i + 1]) bad++;
        _Bool f = !(w & 1);
        if (f && prev) bad++;
        if (f) nf++;
        prev = f;
    }
    *nfree = nf;
    return bad;
}
