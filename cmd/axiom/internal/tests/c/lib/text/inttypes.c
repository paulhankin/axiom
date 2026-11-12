/* <inttypes.h>: each fprintf and fscanf macro is the length modifier of
   its type as <stdint.h> chose the type for the target (read by _Generic
   here) and its conversion; and imaxabs, imaxdiv, strtoimax, strtoumax,
   wcstoimax and wcstoumax. */
#include <errno.h>
#include <inttypes.h>
#include <wchar.h>
#include "../../mix/print.h"
#include <stdlib.h>
#include <string.h>

static int bad;

/* the macro's string is the modifier then the conversion */
static void check(const char *got, const char *mod, const char *conv, const char *name) {
    size_t n = strlen(mod);
    if (strncmp(got, mod, n) || strcmp(got + n, conv)) {
        bad++;
        print_cstr(name);
        print_cstr(got);
    }
}

int main(void) {
    char *end;
    imaxdiv_t q;
    check(PRId8, _Generic((int8_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "PRId8");
    check(PRIu8, _Generic((int8_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "u", "PRIu8");
    check(SCNd8, _Generic((int8_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "SCNd8");
    check(SCNx8, _Generic((int8_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "x", "SCNx8");
    check(PRIX8, _Generic((int8_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "X", "PRIX8");
    check(PRIdLEAST8, _Generic((int_least8_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "PRIdLEAST8");
    check(PRIuLEAST8, _Generic((int_least8_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "u", "PRIuLEAST8");
    check(SCNdLEAST8, _Generic((int_least8_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "SCNdLEAST8");
    check(SCNxLEAST8, _Generic((int_least8_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "x", "SCNxLEAST8");
    check(PRIXLEAST8, _Generic((int_least8_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "X", "PRIXLEAST8");
    check(PRIdFAST8, _Generic((int_fast8_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "PRIdFAST8");
    check(PRIuFAST8, _Generic((int_fast8_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "u", "PRIuFAST8");
    check(SCNdFAST8, _Generic((int_fast8_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "SCNdFAST8");
    check(SCNxFAST8, _Generic((int_fast8_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "x", "SCNxFAST8");
    check(PRIXFAST8, _Generic((int_fast8_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "X", "PRIXFAST8");
    check(PRId16, _Generic((int16_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "PRId16");
    check(PRIu16, _Generic((int16_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "u", "PRIu16");
    check(SCNd16, _Generic((int16_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "SCNd16");
    check(SCNx16, _Generic((int16_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "x", "SCNx16");
    check(PRIX16, _Generic((int16_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "X", "PRIX16");
    check(PRIdLEAST16, _Generic((int_least16_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "PRIdLEAST16");
    check(PRIuLEAST16, _Generic((int_least16_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "u", "PRIuLEAST16");
    check(SCNdLEAST16, _Generic((int_least16_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "SCNdLEAST16");
    check(SCNxLEAST16, _Generic((int_least16_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "x", "SCNxLEAST16");
    check(PRIXLEAST16, _Generic((int_least16_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "X", "PRIXLEAST16");
    check(PRIdFAST16, _Generic((int_fast16_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "PRIdFAST16");
    check(PRIuFAST16, _Generic((int_fast16_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "u", "PRIuFAST16");
    check(SCNdFAST16, _Generic((int_fast16_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "SCNdFAST16");
    check(SCNxFAST16, _Generic((int_fast16_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "x", "SCNxFAST16");
    check(PRIXFAST16, _Generic((int_fast16_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "X", "PRIXFAST16");
    check(PRId32, _Generic((int32_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "PRId32");
    check(PRIu32, _Generic((int32_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "u", "PRIu32");
    check(SCNd32, _Generic((int32_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "SCNd32");
    check(SCNx32, _Generic((int32_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "x", "SCNx32");
    check(PRIX32, _Generic((int32_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "X", "PRIX32");
    check(PRIdLEAST32, _Generic((int_least32_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "PRIdLEAST32");
    check(PRIuLEAST32, _Generic((int_least32_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "u", "PRIuLEAST32");
    check(SCNdLEAST32, _Generic((int_least32_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "SCNdLEAST32");
    check(SCNxLEAST32, _Generic((int_least32_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "x", "SCNxLEAST32");
    check(PRIXLEAST32, _Generic((int_least32_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "X", "PRIXLEAST32");
    check(PRIdFAST32, _Generic((int_fast32_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "PRIdFAST32");
    check(PRIuFAST32, _Generic((int_fast32_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "u", "PRIuFAST32");
    check(SCNdFAST32, _Generic((int_fast32_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "SCNdFAST32");
    check(SCNxFAST32, _Generic((int_fast32_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "x", "SCNxFAST32");
    check(PRIXFAST32, _Generic((int_fast32_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "X", "PRIXFAST32");
    check(PRId64, _Generic((int64_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "PRId64");
    check(PRIu64, _Generic((int64_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "u", "PRIu64");
    check(SCNd64, _Generic((int64_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "SCNd64");
    check(SCNx64, _Generic((int64_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "x", "SCNx64");
    check(PRIX64, _Generic((int64_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "X", "PRIX64");
    check(PRIdLEAST64, _Generic((int_least64_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "PRIdLEAST64");
    check(PRIuLEAST64, _Generic((int_least64_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "u", "PRIuLEAST64");
    check(SCNdLEAST64, _Generic((int_least64_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "SCNdLEAST64");
    check(SCNxLEAST64, _Generic((int_least64_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "x", "SCNxLEAST64");
    check(PRIXLEAST64, _Generic((int_least64_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "X", "PRIXLEAST64");
    check(PRIdFAST64, _Generic((int_fast64_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "PRIdFAST64");
    check(PRIuFAST64, _Generic((int_fast64_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "u", "PRIuFAST64");
    check(SCNdFAST64, _Generic((int_fast64_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "SCNdFAST64");
    check(SCNxFAST64, _Generic((int_fast64_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "x", "SCNxFAST64");
    check(PRIXFAST64, _Generic((int_fast64_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "X", "PRIXFAST64");
    check(PRIdMAX, _Generic((intmax_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "PRIdMAX");
    check(PRIuMAX, _Generic((intmax_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "u", "PRIuMAX");
    check(SCNdMAX, _Generic((intmax_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "SCNdMAX");
    check(SCNxMAX, _Generic((intmax_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "x", "SCNxMAX");
    check(PRIXMAX, _Generic((intmax_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "X", "PRIXMAX");
    check(PRIdPTR, _Generic((intptr_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "PRIdPTR");
    check(PRIuPTR, _Generic((intptr_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "u", "PRIuPTR");
    check(SCNdPTR, _Generic((intptr_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "d", "SCNdPTR");
    check(SCNxPTR, _Generic((intptr_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "x", "SCNxPTR");
    check(PRIXPTR, _Generic((intptr_t)0, signed char: "hh", short: "h", int: "", long: "l", long long: "ll", default: "?"), "X", "PRIXPTR");
    print_i64(bad);
    print_i64(imaxabs(-INTMAX_MAX));
    print_i64(imaxabs(42));
    q = imaxdiv(-17, 5);
    print_i64(q.quot);
    print_i64(q.rem);
    q = imaxdiv(INTMAX_MIN, -1 + 2);
    print_bool(q.quot == INTMAX_MIN && q.rem == 0);
    print_i64(strtoimax(" -9223372036854775808x", &end, 10));
    print_bool(*end == 'x');
    errno = 0;
    print_bool(strtoimax("9223372036854775808", 0, 10) == INTMAX_MAX && errno == ERANGE);
    print_u64(strtoumax("0xffffffffffffffff", 0, 16));
    print_i64(wcstoimax(L"-0x10", 0, 0));
    print_u64(wcstoumax(L"777", 0, 8));
    return 0;
    /* expect: 0 */
    /* expect: 9223372036854775807 */
    /* expect: 42 */
    /* expect: -3 */
    /* expect: -2 */
    /* expect: true */
    /* expect: -9223372036854775808 */
    /* expect: true */
    /* expect: true */
    /* expect: 18446744073709551615 */
    /* expect: -16 */
    /* expect: 511 */
}
