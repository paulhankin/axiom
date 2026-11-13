/* A calculator: a recursive-descent parser of expressions over doubles
   (numbers read by strtod, + - * / % ^, unary minus, parentheses,
   variables, the functions of <math.h>), whose errors leave through
   longjmp to the setjmp of the line being evaluated, so that the next line
   is evaluated afresh; lines come from the standard input (fgets), results
   go out with printf's %.10g. Native only: WebAssembly has no setjmp. */
#include <ctype.h>
#include <math.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static jmp_buf on_error;
static const char *p;
static const char *message;
static double vars[26];
static int depth;

static void fail(const char *why) {
    message = why;
    longjmp(on_error, 1);
}

static void skip(void) {
    while (isspace((unsigned char)*p))
        p++;
}

static double expr(void);

static double primary(void) {
    double v;
    skip();
    if (*p == '(') {
        p++;
        if (++depth > 50)
            fail("too deep");
        v = expr();
        depth--;
        skip();
        if (*p != ')')
            fail("missing )");
        p++;
        return v;
    }
    if (*p == '-') {
        p++;
        return -primary();
    }
    if (isalpha((unsigned char)*p)) {
        char name[16];
        int n = 0;
        while (isalpha((unsigned char)*p) && n < 15)
            name[n++] = *p++;
        name[n] = 0;
        skip();
        if (*p == '(') {
            double a;
            p++;
            a = expr();
            skip();
            if (*p != ')')
                fail("missing )");
            p++;
            if (!strcmp(name, "sqrt")) {
                if (a < 0)
                    fail("sqrt of a negative number");
                return sqrt(a);
            }
            if (!strcmp(name, "exp"))
                return exp(a);
            if (!strcmp(name, "log")) {
                if (a <= 0)
                    fail("log of a nonpositive number");
                return log(a);
            }
            if (!strcmp(name, "sin"))
                return sin(a);
            if (!strcmp(name, "floor"))
                return floor(a);
            fail("unknown function");
        }
        if (n != 1)
            fail("unknown variable");
        return vars[name[0] - (islower((unsigned char)name[0]) ? 'a' : 'A')];
    }
    {
        char *end;
        v = strtod(p, &end);
        if (end == p)
            fail("a number expected");
        p = end;
        return v;
    }
}

static double power(void) {
    double b = primary();
    skip();
    if (*p == '^') {
        p++;
        return pow(b, power());
    }
    return b;
}

static double term(void) {
    double v = power();
    for (;;) {
        skip();
        if (*p == '*') {
            p++;
            v *= power();
        } else if (*p == '/' || *p == '%') {
            char op = *p++;
            double d = power();
            if (d == 0)
                fail("division by zero");
            v = op == '/' ? v / d : fmod(v, d);
        } else {
            return v;
        }
    }
}

static double expr(void) {
    double v = term();
    for (;;) {
        skip();
        if (*p == '+') {
            p++;
            v += term();
        } else if (*p == '-') {
            p++;
            v -= term();
        } else {
            return v;
        }
    }
}

int main(void) {
    char line[256];
    int n = 0, errors = 0;
    while (fgets(line, sizeof line, stdin) != NULL) {
        volatile int var = -1;
        line[strcspn(line, "\n")] = 0;
        n++;
        p = line;
        depth = 0;
        if (setjmp(on_error) != 0) {
            errors++;
            printf("%d: error: %s at column %d\n", n, message, (int)(p - line) + 1);
            continue;
        }
        skip();
        if (isalpha((unsigned char)p[0]) && p[1] == '=') {
            var = p[0] - 'a';
            p += 2;
        }
        {
            double v = expr();
            skip();
            if (*p != 0)
                fail("unexpected text");
            if (var >= 0)
                vars[var] = v;
            printf("%d: %.10g\n", n, v);
        }
    }
    printf("%d lines, %d errors\n", n, errors);
    return 0;
}
// native-only
// stdin: 1 + 2 * 3
// stdin: x=2^10
// stdin: (x - 24) / 10 % 7
// stdin: sqrt(2) * sqrt(2)
// stdin: 1 / 0
// stdin: y=log(exp(3.25)) + floor(-2.5)
// stdin: y * (1 + 2
// stdin: sqrt(-1)
// stdin: 2 ^ 3 ^ 2
// stdin: 0x10 + 1e2 + .5
// stdin: foo(1)
// stdin: x + y + 3 3
// stdin: -(-(-4))

// expect: 1: 7
// expect: 2: 1024
// expect: 3: 2
// expect: 4: 2
// expect: 5: error: division by zero at column 6
// expect: 6: 0.25
// expect: 7: error: missing ) at column 11
// expect: 8: error: sqrt of a negative number at column 9
// expect: 9: 512
// expect: 10: 116.5
// expect: 11: error: unknown function at column 7
// expect: 12: error: unexpected text at column 11
// expect: 13: -4
// expect: 13 lines, 5 errors
