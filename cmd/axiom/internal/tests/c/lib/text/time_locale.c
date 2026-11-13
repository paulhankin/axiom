/* strftime in every locale setlocale accepts writes the "C" locale's
   names and formats (each locale has the "C" locale's LC_TIME, docs/c-text.md),
   and so does wcsftime. */
#include <stdio.h>
#include <time.h>
#include <locale.h>
#include <wchar.h>

int main(void) {
    static const char *const names[] = {"C", "C.UTF-8", "en_GB.UTF-8", "POSIX"};
    struct tm tm = {0};
    char buf[128];
    wchar_t w[128];
    int k;
    tm.tm_year = 99;
    tm.tm_mon = 11;
    tm.tm_mday = 31;
    tm.tm_hour = 23;
    tm.tm_min = 59;
    tm.tm_sec = 58;
    tm.tm_wday = 5;
    tm.tm_yday = 364;
    for (k = 0; k < 4; k++) {
        if (setlocale(LC_ALL, names[k]) == NULL)
            continue;
        strftime(buf, sizeof buf, "%c|%x|%X|%A %a %B %b %p|%r", &tm);
        wcsftime(w, 128, L"%A", &tm);
        printf("[%s] %s %ls\n", names[k], buf, w);
    }
    return 0;
}
// expect: [C] Fri Dec 31 23:59:58 1999|12/31/99|23:59:58|Friday Fri December Dec PM|11:59:58 PM Friday
// expect: [C.UTF-8] Fri Dec 31 23:59:58 1999|12/31/99|23:59:58|Friday Fri December Dec PM|11:59:58 PM Friday
// expect: [en_GB.UTF-8] Fri Dec 31 23:59:58 1999|12/31/99|23:59:58|Friday Fri December Dec PM|11:59:58 PM Friday
// expect: [POSIX] Fri Dec 31 23:59:58 1999|12/31/99|23:59:58|Friday Fri December Dec PM|11:59:58 PM Friday
