/* __axc_days_from_civil - the days since 1970-01-01 of a date of the
   proleptic Gregorian calendar (axc/text.h), by H. Hinnant's algorithm
   ("chrono-Compatible Low-Level Date Algorithms", 2013): years counted
   from March, in eras of 400 years (146097 days). */
#include <axc/text.h>

long long __axc_days_from_civil(long long y, int m, int d) {
    long long era, yoe, doy, doe;
    if (m <= 2)
        y--;
    era = (y >= 0 ? y : y - 399) / 400;
    yoe = y - era * 400;
    doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
    doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}
