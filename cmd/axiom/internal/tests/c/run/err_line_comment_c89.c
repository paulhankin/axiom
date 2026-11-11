/* flags: --c-std=c89 */
/* expect-error: C2003 */
/* '//' comments are C99's: before, '/' and '/' written together (never
   valid C) are reported as one, and the rest of the line is skipped. */
int main(void) {
    int a = 4; // the comment
    return a;
}
