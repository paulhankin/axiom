/* flags: --c-std=c23 */
/* expect-error: C3077 */
/* deprecated does not apply to labels (maybe_unused does). */
int main(void) {
    goto out;
[[deprecated]] out:
    return 0;
}
