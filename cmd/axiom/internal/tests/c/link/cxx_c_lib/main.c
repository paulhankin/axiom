// With the compiler's C library (the default, --c-lib=axiom), a program
// linked with the shared C++ runtime library, which is built against the
// system's C library (its streams, locales and the multibyte state of its
// own layout), is an error: the program would have two C libraries.
// needs: c++
// cxx-exceptions
// expect-error: E9004
// expect-stderr: is built against the system's C library
int cxx_top(int);

int main(void) {
    return cxx_top(1);
}
