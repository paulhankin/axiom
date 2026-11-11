/* flags: --c-std=c23 */
/* expect-error: C3077 */
int main(void) {
    [[deprecated]];
    return 0;
}
