/* flags: --c-std=c23 */
/* C23 expressions: true, false, nullptr, alignof. */
int f(void) {
    bool t = true, u = false;
    int *p = nullptr;
    return t + u + (p == nullptr) + (int)alignof(int) + (int)_Alignof(long);
}
