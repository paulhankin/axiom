inline int __attribute__((noinline)) shared_inline(int x) {
    static int calls = 0;
    calls++;
    return x * 2 + calls;
}
extern "C" int cpp_two(int x) { return shared_inline(x); }
