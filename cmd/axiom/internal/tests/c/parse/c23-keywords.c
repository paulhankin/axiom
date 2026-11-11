/* flags: --c-std=c23 */
/* C23 keywords: typeof, typeof_unqual, constexpr, auto, _BitInt, bool,
   alignas, thread_local. */
constexpr int N = 4;
alignas(8) char buf[N];
thread_local int tl;
bool flag;
unsigned _BitInt(12) bits;
int f(int x) {
    auto y = x + 1;
    static auto z = 2;
    typeof(y) a = y;
    typeof(int *) p = &a;
    typeof_unqual(const int) b = *p;
    return b + z;
}
