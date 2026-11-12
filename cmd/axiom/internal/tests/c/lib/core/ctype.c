/* <ctype.h> (C17 7.4) in the "C" locale: each function over EOF (-1) and every
   value of unsigned char, as the counts of each class (7.4.1: the letters
   and digits of 5.2.1, the six white-space characters, the 95 printing
   characters 0x20..0x7E, the 33 control characters 0..0x1F and 0x7F; no
   byte past 127 is in any class), and the case mappings (7.4.2). */
#include <ctype.h>
#include "print.h"

static int count(int (*f)(int)) {
    int c, n = 0;
    for (c = -1; c <= 255; c++)
        n += f(c) != 0;
    return n;
}

int main(void) {
    int c, lower = 0, upper = 0, same = 1;
    print_i64(count(isalnum)); /* expect: 62 */
    print_i64(count(isalpha)); /* expect: 52 */
    print_i64(count(isblank)); /* expect: 2 */
    print_i64(count(iscntrl)); /* expect: 33 */
    print_i64(count(isdigit)); /* expect: 10 */
    print_i64(count(isgraph)); /* expect: 94 */
    print_i64(count(islower)); /* expect: 26 */
    print_i64(count(isprint)); /* expect: 95 */
    print_i64(count(ispunct)); /* expect: 32 */
    print_i64(count(isspace)); /* expect: 6 */
    print_i64(count(isupper)); /* expect: 26 */
    print_i64(count(isxdigit)); /* expect: 22 */
    for (c = -1; c <= 255; c++) {
        int l = tolower(c), u = toupper(c);
        if (l != c)
            lower += l - c;
        if (u != c)
            upper += c - u;
        if (isalpha(c) != (isupper(c) || islower(c)) || isalnum(c) != (isalpha(c) || isdigit(c)) ||
            isgraph(c) != (isalnum(c) || ispunct(c)) || isprint(c) != (isgraph(c) || c == ' ') ||
            (c >= 128 && (isprint(c) || iscntrl(c) || isspace(c))))
            same = 0;
    }
    print_i64(lower); /* expect: 832 */
    print_i64(upper); /* expect: 832 */
    print_bool(same); /* expect: true */
    print_bool(isspace('\t') && isspace('\v') && isspace('\f') && isspace('\r') && isspace('\n') && !isspace('\b'));
    /* expect: true */
    print_bool(isblank('\t') && !isblank('\n')); /* expect: true */
    print_bool(isxdigit('f') && isxdigit('F') && !isxdigit('g')); /* expect: true */
    print_i64(tolower(-1)); /* expect: -1 */
    print_i64(toupper('a') + tolower('Z') + toupper('1')); /* expect: 236 */
    print_i64(isalpha(1000) + isdigit(-2) + toupper(-200)); /* expect: -200 */
    return 0;
}
