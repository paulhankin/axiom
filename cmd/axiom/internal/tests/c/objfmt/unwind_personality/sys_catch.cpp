// C++: a destructor and a handler (the compact entry's personality routine
// and LSDA).
extern "C" void sys_inner(int *);

struct Guard {
    int *p;
    ~Guard() { p[0]++; }
};

extern "C" int sys_catch(int *s) {
    try {
        Guard g{s};
        sys_inner(s);
    } catch (int e) {
        return e;
    }
    return s[0];
}
