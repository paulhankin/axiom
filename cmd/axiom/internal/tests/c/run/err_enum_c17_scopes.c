/* expect-error: C3022 */
/* Before C23, enumerations of the same tag declared in different scopes are
   different types, whatever their constants (C17 6.7.2.3p5). */
enum G { X, Y } gg;
int main(void) {
    enum G { X2, Y2 } h = X2;
    enum G *p = &gg;
    return h + (p != 0);
}
