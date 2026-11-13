/* flags: --c-std=gnu23 */
/* expect-error: C3022 */
/* Enumerations with the same tag whose constants differ only above the
   first 64 bits are not compatible (C23 6.2.7p1). */
enum G { X = (__int128)1 << 64 } gg;
int main(void) {
    enum G { X = (__int128)1 << 65 } h = X;
    enum G *p = &gg;
    return (int)h + (p != 0);
}
