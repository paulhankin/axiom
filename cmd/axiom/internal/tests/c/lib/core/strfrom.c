/* strfromd, strfromf, strfroml (C23 7.24.1.3): as snprintf with the format
   "%[.precision]conversion" would write, the length of the whole result,
   at most n bytes with the null; another format (a flag, a width, a length
   modifier) writes nothing and gives -1, where the standard leaves it
   undefined. */
/* flags: --c-std=c23 */
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    char buf[64];
    int n;
    n = strfromd(buf, sizeof buf, "%.3f", 3.14159);
    printf("%d %s\n", n, buf);
    n = strfromd(buf, sizeof buf, "%g", 1e-10);
    printf("%d %s\n", n, buf);
    n = strfromd(buf, sizeof buf, "%a", 0.5);
    printf("%d %s\n", n, buf);
    n = strfromf(buf, sizeof buf, "%.2e", 1234.5f);
    printf("%d %s\n", n, buf);
    n = strfroml(buf, sizeof buf, "%.1F", 2.25L);
    printf("%d %s\n", n, buf);
    n = strfromd(buf, 4, "%f", 123.456);
    printf("%d %s\n", n, buf);
    n = strfromd(NULL, 0, "%.20e", 1.0);
    printf("%d\n", n);
    printf("%d %d %d\n", strfromd(buf, sizeof buf, "%5f", 1.0), strfromd(buf, sizeof buf, "%d", 1.0),
           strfromd(buf, sizeof buf, "%Lf", 1.0));
    return 0;
}
/* expect: 5 3.142 */
/* expect: 5 1e-10 */
/* expect: 6 0x1p-1 */
/* expect: 8 1.23e+03 */
/* expect: 3 2.2 */
/* expect: 10 123 */
/* expect: 26 */
/* expect: -1 -1 -1 */
