/* flags: --c-std=c23 */
/* expect-error: C3077 */
/* No standard attribute appertains to a statement. */
int main(void) {
    [[maybe_unused]] return 0;
}
