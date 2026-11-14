/* flags: --c-std=gnu89 */
/* expect-error: C2100 */
/* In gnu89 a later standard's keyword with an ordinary spelling stays an
   error where it is used as one (a program may use the name for its own):
   only the reserved spellings are taken. */
int main(void) {
    bool b = 0;
    return b;
}
