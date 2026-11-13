/* The C unit of profile_with_c/: some work to sample. */
long long c_work(long long n) {
    volatile long long s = 0;
    for (long long i = 0; i < n; i++)
        s += i ^ (s >> 3);
    return s | 1;
}
