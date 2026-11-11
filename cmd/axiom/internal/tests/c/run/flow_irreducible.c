/* goto making control flow irreducible (cycles with more than one entry):
   between two blocks, into the bodies of while, do and for loops, into
   if arms and switch bodies, nested cycles, a state machine. (The
   WebAssembly backend structures reducible control flow only: these get
   a dispatch block.) */
#include "host.h"
/* a cycle of two blocks entered at either */
static int two_entries(int x) {
    int n = 0, i = 0;
    if (x) goto b;
a:
    n += 1;
    i++;
b:
    n += 10;
    if (i < 5) goto a;
    return n;
}
/* into a while loop's body, past its test */
static int into_while(int k) {
    int i = k, s = 0;
    if (k > 2) goto inside;
    while (i < 10) {
        s += 100;
    inside:
        s += i;
        i++;
    }
    return s;
}
/* into the arms of an if, and from one arm into the other */
static int into_if(int x) {
    int r = 0;
    if (x == 1) goto then_part;
    if (x == 2) goto else_part;
    if (x > 5) {
    then_part:
        r += 1;
        if (x == 1) goto else_part;
    } else {
    else_part:
        r += 10;
        if (x == 2) goto then_part;
    }
    return r;
}
/* into a switch body from outside, and from case to case backwards */
static int into_switch(int x, int y) {
    int r = 0, k = 0;
    if (y) goto mid;
    switch (x) {
    case 0:
        r += 1;
    mid:
        r += 2;
        if (++k < 3) goto back;
        break;
    case 1:
    back:
        r += 4;
        goto mid;
    default:
        r = -1;
    }
    return r;
}
/* into do and for bodies (skipping a for statement's first clause) */
static int into_do_for(int x) {
    int s = 0, i = 0, j = 7;
    if (x & 1) goto in_do;
    if (x & 2) goto in_for;
    do {
        s += 1000;
    in_do:
        s += 1;
    } while (++i < 3);
    for (j = 0; j < 4; j++) {
        s += 10;
    in_for:
        s += j;
    }
    return s;
}
/* nested cycles each with two entries */
static int nested(int x) {
    int s = 0, i = 0, j = 0;
    if (x) goto inner2;
outer:
    s += 1;
    j = 0;
inner1:
    s += 10;
inner2:
    s += 100;
    if (++j < 3) goto inner1;
    if (++i < 3) goto outer;
    return s;
}
/* a state machine of gotos */
static int machine(const char *p) {
    int n = 0;
start:
    if (*p == 0) return n;
    if (*p == 'a') goto sa;
    if (*p == 'b') goto sb;
    p++;
    goto start;
sa:
    n = n * 2 + 1;
    p++;
    if (*p == 'b') goto sb;
    goto start;
sb:
    n = n * 3;
    p++;
    if (*p == 'a') goto sa;
    if (*p == 'b') goto sb;
    goto start;
}
int main(void) {
    print_i64(two_entries(0) * 1000 + two_entries(1));
    print_i64(into_while(0) * 10000 + into_while(5));
    for (int x = 0; x < 8; x++) print_i64(into_if(x) * 100 + into_switch(x % 3, x & 1) * 10 + into_do_for(x) % 10);
    print_i64(into_do_for(1) + into_do_for(2) * 100000);
    print_i64(nested(0) * 10000 + nested(1));
    print_i64(machine("abxaabbaba"));
    return 0;
}

/* expect: 55065 */
/* expect: 10450435 */
/* expect: 1159 */
/* expect: 1249 */
/* expect: 1097 */
/* expect: 1149 */
/* expect: 1189 */
/* expect: 1149 */
/* expect: 257 */
/* expect: 249 */
/* expect: 702049 */
/* expect: 9930982 */
/* expect: 1627 */
