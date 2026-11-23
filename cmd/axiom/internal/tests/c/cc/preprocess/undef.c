#if defined(__x86_64__) || defined(__aarch64__) || defined(__linux__) || defined(__GNUC__) || defined(unix)
#error "-undef: a target's macro, or the GNU compilers', is defined"
#endif
#if !defined(__STDC__) || !defined(__STDC_VERSION__)
#error "-undef: the standard's macros are not defined"
#endif
int main(void) { return 6; }
