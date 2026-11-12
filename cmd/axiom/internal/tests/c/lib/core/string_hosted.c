/* The hosted functions of <string.h> (C17 7.24): strerror's messages (one
   per error number, constant), strcoll and strxfrm in the "C" locale (the
   order of the bytes as unsigned char; strxfrm's length and what it writes
   when the array is too small). */
#include <errno.h>
#include <string.h>
#include "print.h"

int main(void) {
    char buf[8] = "xxxxxxx";
    print_cstr(strerror(EDOM)); /* expect: Argument outside the domain */
    print_cstr(strerror(ERANGE)); /* expect: Result out of range */
    print_cstr(strerror(EILSEQ)); /* expect: Invalid or incomplete multibyte or wide character */
    print_cstr(strerror(0)); /* expect: No error */
    print_cstr(strerror(-5)); /* expect: Unknown error */
    print_bool(strerror(ENOMEM) == strerror(ENOMEM)); /* expect: true */
    print_bool(strcoll("abc", "abd") < 0 && strcoll("b", "a") > 0 && strcoll("x", "x") == 0); /* expect: true */
    print_bool(strcoll("\xe9", "z") > 0 && strcoll("", "a") < 0); /* expect: true */
    print_i64((long long)strxfrm(buf, "hello", sizeof buf)); /* expect: 5 */
    print_cstr(buf); /* expect: hello */
    print_i64((long long)strxfrm(buf, "a longer string", sizeof buf)); /* expect: 15 */
    print_cstr(buf); /* expect: hello */
    print_i64((long long)strxfrm(NULL, "count", 0)); /* expect: 5 */
    return 0;
}
