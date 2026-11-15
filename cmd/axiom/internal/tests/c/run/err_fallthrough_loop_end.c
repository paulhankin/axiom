/* C23: a fallthrough declaration at the end of a loop's body is followed by
   the loop's next iteration, not by the case label after the loop, even in
   a switch statement. */
/* flags: --c-std=c23 */
/* expect-error: C3077 */
int f(int x) {
    switch (x) {
    case 1:
        for (int i = 0; i < 3; i++) {
            x += i;
            [[fallthrough]];
        }
    case 2:
        return x;
    }
    return 0;
}
int main(void) { return f(1); }
