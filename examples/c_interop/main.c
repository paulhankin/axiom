// Uses the Phalange library in stats.ph through its generated header.
#include <stdio.h>

#include "stats.h"

static void report(size_t n) { printf("sorted %zu values\n", n); }

int main(void) {
    double xs[] = {3.5, -1.0, 8.25, 2.0, 0.5};
    size_t n = sizeof xs / sizeof xs[0];
    Summary s;
    if (!stats_summarize(xs, n, &s)) return 1;
    printf("count=%zu min=%g max=%g mean=%g\n", s.count, s.min, s.max, s.mean);
    stats_sort(xs, n, report);
    for (size_t i = 0; i < n; i++) printf("%g ", xs[i]);
    printf("\n");
    return 0;
}
