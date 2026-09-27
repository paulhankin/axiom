// Host functions for running the test suite on the native target.
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_i64(int64_t x) { printf("%" PRId64 "\n", x); }
void print_u64(uint64_t x) { printf("%" PRIu64 "\n", x); }
void print_bool(int b) { puts(b ? "true" : "false"); }
void print_str(const char *p, size_t n) { printf("%.*s\n", (int)n, p); }

// Mimic JavaScript's Number#toString for the common cases.
void print_f64(double x) {
    if (isnan(x)) { puts("NaN"); return; }
    if (isinf(x)) { puts(x > 0 ? "Infinity" : "-Infinity"); return; }
    if (x == 0) { puts("0"); return; }
    char buf[64];
    for (int p = 1; p <= 17; p++) {
        snprintf(buf, sizeof buf, "%.*g", p, x);
        if (strtod(buf, NULL) == x) break;
    }
    char *e = strchr(buf, 'e');
    if (e && fabs(x) >= 1e-6 && fabs(x) < 1e21) {
        for (int p = 1; p <= 17; p++) {
            snprintf(buf, sizeof buf, "%.*f", p, x);
            if (strtod(buf, NULL) == x) break;
        }
        char *d = strchr(buf, '.');
        if (d) {
            char *z = buf + strlen(buf) - 1;
            while (z > d && *z == '0') *z-- = 0;
            if (z == d) *z = 0;
        }
    } else if (e) {
        // JS style exponent: 1e+21, 1.5e-7
        char mant[32];
        int ex = atoi(e + 1);
        snprintf(mant, sizeof mant, "%.*s", (int)(e - buf), buf);
        snprintf(buf, sizeof buf, "%se%c%d", mant, ex < 0 ? '-' : '+', abs(ex));
    }
    puts(buf);
}

void run(void);
int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    run();
    return 0;
}
