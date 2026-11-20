/* Recursive functions: naive Fibonacci, Ackermann, and a recursive n-queens count. */
#include <stdio.h>
static long fib(int n) { return n < 2 ? n : fib(n - 1) + fib(n - 2); }
static int ack(int m, int n) { return m == 0 ? n + 1 : n == 0 ? ack(m - 1, 1) : ack(m - 1, ack(m, n - 1)); }
static int queens(int n, int row, unsigned cols, unsigned d1, unsigned d2) {
    if (row == n) return 1;
    int c = 0;
    unsigned avail = ~(cols | d1 | d2) & ((1u << n) - 1);
    while (avail) {
        unsigned b = avail & -avail; avail -= b;
        c += queens(n, row + 1, cols | b, (d1 | b) << 1, (d2 | b) >> 1);
    }
    return c;
}
int main(void) {
    printf("recur: %ld %d %d\n", fib(35), ack(2, 2000) + ack(3, 8), queens(12, 0, 0, 0, 0));
    return 0;
}
