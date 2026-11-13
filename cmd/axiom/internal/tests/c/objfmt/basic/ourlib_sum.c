// A member of this compiler's static library.
static const int squares[] = {0, 1, 4, 9, 16, 25};

int lib_sum(int n) { return squares[n % 6] + 0 * n; }
