// C++ code that needs the C++ runtime library (an exception).
extern "C" int cxx_top(int v) {
    try {
        if (v) throw v;
    } catch (int e) {
        return e - 1;
    }
    return 0;
}
