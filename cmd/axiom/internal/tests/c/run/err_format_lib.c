/* flags: --c-std=c17 */
/* wasm-only */
/* The C library's declarations carry the format attribute in every mode
   (here C17): printf's, scanf's and strftime's formats are checked, and
   #pragma GCC diagnostic names the checks -Wformat (here errors). */
/* expect-error: C4130 */
/* expect-error: C4131 */
/* expect-error: C4133 */
#include <stdio.h>
#include <time.h>
#pragma GCC diagnostic error "-Wformat"
int main(void) {
    char b[40];
    struct tm t = {0};
    int n;
    printf("%s\n", 1);
    scanf("%d %d", &n);
    strftime(b, sizeof b, "%Q", &t);
    return 0;
}
