/* Precedence and associativity of the binary operators. */
int f(int a, int b, int c) {
    a = a * b + c << 1 < b == c & a ^ b | c && a || b;
    a = a || b && c | a ^ b & c == a < b << c + a * b;
    a = a - b - c;
    a = a / b % c;
    a = b = c;
    a += b -= c *= 2;
    return a ? b : c ? a : b;
}
