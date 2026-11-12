/* flags: --c-std=gnu23 */
/* expect-error: C3901 */
/* a case range in a switch on a type wider than 64 bits is not supported */
int f(__int128 x) {
    switch (x) {
    case 1 ... 5: return 1;
    }
    return 0;
}
int main(void) { return f(2); }
