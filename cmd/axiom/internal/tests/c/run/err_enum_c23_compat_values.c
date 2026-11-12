/* flags: --c-std=c23 */
/* expect-error: C3022 */
/* Enumerations with the same tag whose constants have other values are not
   compatible (C23 6.2.7p1). */
enum G { X, Y } gg;
int main(void) {
    enum G { X = 1, Y } h = X;
    enum G *p = &gg;
    return h + (p != 0);
}
