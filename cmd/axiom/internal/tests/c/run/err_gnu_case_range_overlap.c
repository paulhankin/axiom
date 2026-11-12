/* flags: --c-std=gnu11 */
/* expect-error: C3041 */
/* expect-warning: C4120 */
int main(void) {
    int x = 3;
    switch (x) {
    case 1 ... 5: return 1;
    case 4: return 2;
    case 9 ... 7: return 3;
    }
    return 0;
}
