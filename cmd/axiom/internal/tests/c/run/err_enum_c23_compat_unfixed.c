/* flags: --c-std=c23 */
/* expect-error: C3022 */
/* An enumeration with a fixed underlying type is not compatible with one of
   the same tag and constants without one (C23 6.2.7p1). */
enum H : unsigned { M } hh;
int main(void) {
    enum H { M } z = M;
    enum H *p = &hh;
    return z + (p != 0);
}
