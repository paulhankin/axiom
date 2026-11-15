/* C23: a fallthrough declaration that ends an inner switch statement's body
   leaves that switch; the case label after it belongs to the outer one. */
/* flags: --c-std=c23 */
/* expect-error: C3077 */
int f(int x, int y) {
    switch (x) {
    case 1:
        switch (y) {
        case 0:
            y++;
            [[fallthrough]];
        }
    case 2:
        return y;
    }
    return 0;
}
int main(void) { return f(1, 0); }
