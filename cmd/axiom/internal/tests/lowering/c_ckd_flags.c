/* The GNU overflow builtins with operands and result of one type are that
   type's checked arithmetic (cgen_va_arg.ax: ckd_same): one multiply,
   add or sub, and a jump on its flags; the overflow as a value, a setcc of
   them. */
// lower: --target=native --cpu=x86-64 -O2 --c-freestanding --c-std=gnu17 | \bi?mul\s | 5
// lower: --target=native --cpu=x86-64 -O2 --c-freestanding --c-std=gnu17 | \bimul\s+%\w+,%\w+\n[^\n]*\bjn?o\b | 2
// lower: --target=native --cpu=x86-64 -O2 --c-freestanding --c-std=gnu17 | \bmul\s+%\w+\n[^\n]*\bj(b|ae)\b | 2
// lower: --target=native --cpu=x86-64 -O2 --c-freestanding --c-std=gnu17 | \badd\s+%\w+,%\w+\n[^\n]*\bjn?o\b | 1
// lower: --target=native --cpu=x86-64 -O2 --c-freestanding --c-std=gnu17 | \bsub\s+%\w+,%\w+\n[^\n]*\bj(b|ae)\b | 1
// lower: --target=native --cpu=x86-64 -O2 --c-freestanding --c-std=gnu17 | \bmul\s+%\w+\n([^\n]*\bmov\s[^\n]*\n)?[^\n]*\bsetb\b | 1
// lower: --target=native --cpu=x86-64 -O2 --c-freestanding --c-std=gnu17 | \bset[a-z]+\s | 1

long smul(long a, long b) {
    long r;
    if (__builtin_mul_overflow(a, b, &r))
        return 1;
    return r;
}

unsigned long umul(unsigned long a, unsigned long b) {
    unsigned long r;
    if (__builtin_mul_overflow(a, b, &r))
        return 1;
    return r;
}

int smul32(int a, int b) {
    int r;
    if (__builtin_mul_overflow(a, b, &r))
        return 1;
    return r;
}

unsigned umul32(unsigned a, unsigned b) {
    unsigned r;
    if (__builtin_mul_overflow(a, b, &r))
        return 1;
    return r;
}

int sadd(int a, int b) {
    int r;
    if (__builtin_add_overflow(a, b, &r))
        return 1;
    return r;
}

unsigned long usub(unsigned long a, unsigned long b) {
    unsigned long r;
    if (__builtin_sub_overflow(a, b, &r))
        return 1;
    return r;
}

/* (the overflow as a value) */
unsigned long count(unsigned long a, unsigned long b, unsigned long *out) {
    return __builtin_mul_overflow(a, b, out) + 10;
}
