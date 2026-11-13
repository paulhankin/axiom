// The platform toolchain's object importing from DLLs (__imp_ names), the
// DLLs given by -l and found in the -L directories (their export tables).
// (Wine's DLLs are x64 ones)
// targets: x86_64-windows
// wine-dlls: shlwapi kernel32
// expect: file.txt 4
#include <stdio.h>

const char *base(const char *p);
int len(const char *p);

int main(void) {
    printf("%s %d\n", base("C:\\dir\\file.txt"), len("abcd"));
    return 0;
}
