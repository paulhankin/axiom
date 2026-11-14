// C++ that throws through this compiler's C code (main.c) and catches above
// it: an int, and an object of a derived class caught by its base, by
// reference (kind 1); kind 2 throws nothing.
extern "C" void our_top(int *, int, int);
extern "C" void our_mid(int *, int, int);

struct Base {
    int v;
    virtual ~Base() {}
};
struct Derived : Base {};

// counts the frames left by an exception (s[1])
struct Guard {
    int *s;
    ~Guard() { s[1]++; }
};

extern "C" void cxx_throw(int kind, int v) {
    if (kind == 0) throw v;
    if (kind == 1) {
        Derived d;
        d.v = 7;
        throw d;
    }
}

extern "C" void cxx_mid(int *s, int n, int kind) {
    Guard g{s};
    our_mid(s, n, kind);
    s[1]--; // (returned: the guard's count is not the exception's)
}

extern "C" int cxx_top(int *s, int n, int kind) {
    try {
        Guard g{s};
        our_top(s, n, kind);
        s[1]--;
    } catch (int e) {
        return e;
    } catch (Base &b) {
        return b.v;
    }
    return -1;
}
