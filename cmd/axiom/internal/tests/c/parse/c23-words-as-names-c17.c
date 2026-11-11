/* flags: --c-std=c17 */
/* Before C23, true, false and nullptr are ordinary identifiers. */
int true = 1, false, nullptr;
int f(void) {
    return true + false + nullptr;
}
