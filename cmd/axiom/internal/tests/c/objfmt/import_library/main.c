// The platform toolchain's object importing from DLLs, bound by import
// libraries (their short import records), one of them naming a function
// with no dllimport (called through its thunk).
// targets: x86_64-windows aarch64-windows
// dlltool: shlwapi PathFindFileNameA PathIsRelativeA
// dlltool: kernel32 lstrlenA
// expect: file.txt 4 1
#include <stdio.h>

const char *base(const char *p);
int len(const char *p);
int PathIsRelativeA(const char *p);

int main(void) {
    printf("%s %d %d\n", base("C:\\dir\\file.txt"), len("abcd"), PathIsRelativeA("a\\b") != 0);
    return 0;
}
