/* String literals and character constants: escapes, concatenation, wide
   and UTF literals, __func__. */
#include "host.h"
#include <stddef.h>

static size_t len(const char *s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

static int show(void) {
    print_str(__func__, sizeof __func__ - 1);
    return (int)sizeof __func__;
}

int main(void) {
    const char *s = "tab\there" " joined";
    wchar_t w[] = L"wide";
    char e[] = "\x41\102\n\0end";
    print_i64(len(s));
    print_str(s, len(s));
    print_i64(sizeof w / sizeof w[0]);
    print_i64(w[1]);
    print_i64(sizeof e);
    print_i64(e[0] + e[1] + e[2] + e[3]);
    print_i64('\n' + '\t' + '\\' + '\'' + '"' + '\0');
    print_i64('A' == 65);
    print_i64(L'z');
    print_i64(show());
    print_str(__func__, 4);
    print_i64("hello"[4]);
    print_i64(*"x");
    print_i64(sizeof("ab" "cd"));
    return 0;
}

/* expect: 15 */
/* expect: tab	here joined */
/* expect: 5 */
/* expect: 105 */
/* expect: 8 */
/* expect: 141 */
/* expect: 184 */
/* expect: 1 */
/* expect: 122 */
/* expect: show */
/* expect: 5 */
/* expect: main */
/* expect: 111 */
/* expect: 120 */
/* expect: 5 */
