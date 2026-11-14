/* A loop whose switch arms call functions, with more values live around it
   than registers survive the calls: a value's piece in the loop that would
   be reloaded after each call more often than the loop reads it stays in
   its slot, read where it is used (mach.ax greedy), rather than all of them
   being reloaded after every call. */
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bmov\s+-0x[0-9a-f]+\(%rbp\),%r\w+\n | 11
long op_a(long x);
long op_b(long x, long y);
void op_c(long *p);

long mix(const unsigned char *code, long n) {
    long a = 0, b = 1, c = 2, d = 3, e = 4, f = 5, g = 6, h = 7, cnt = 0;
    for (long i = 0; i < n; i++) {
        switch (code[i] & 7) {
        case 0: a = op_a(a + i); break;
        case 1: b = op_b(b, i); cnt++; break;
        case 2: op_c(&cnt); break;
        case 3: c += d; d ^= i; break;
        case 4: e = e * 3 + f; break;
        case 5: f += g; g -= h; h += 1; break;
        case 6: a += b + c; break;
        default: h ^= a; break;
        }
    }
    return a + b + c + d + e + f + g + h + cnt;
}
