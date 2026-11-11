/* flags: --c-std=c23 */
/* expect-error: C3035 */
constexpr int k[2] = { 1, 2 };
static_assert(k[0] == 1);
int main(void) { return 0; }
