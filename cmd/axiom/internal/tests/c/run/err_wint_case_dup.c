/* flags: --c-std=gnu23 */
/* expect-error: C3041 */
int f(unsigned __int128 x) {
    switch (x) {
    case (unsigned __int128)1 << 100: return 1;
    case (unsigned __int128)2 << 99: return 2;
    }
    return 0;
}
int main(void) { return f(2); }
