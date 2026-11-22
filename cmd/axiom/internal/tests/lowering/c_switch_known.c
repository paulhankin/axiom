/* A switch on a value whose possible values are known (switches.ax:
   value_range) needs no range check before its jump table: a state
   machine's state, phis and selects of the constants 0 to 5 (its cases).
   The only compares left are the loop's and the characters'. */
// lower: --target=native --cpu=x86-64 -O2 --c-freestanding | \bcmp\s+\$0x[56],%e | 0
// lower: --target=native --cpu=x86-64 -O2 --c-freestanding | \bjmp\s+\*%r | 1
int states(const char *s, int n) {
    int st = 0, words = 0;
    for (int i = 0; i < n; i++) {
        char c = s[i];
        switch (st) {
        case 0: st = c == ' ' ? 0 : c == '"' ? 2 : 1; break;
        case 1: if (c == ' ') { words++; st = 0; } else if (c == ',') st = 4; break;
        case 2: if (c == '"') st = 3; break;
        case 3: st = c == '"' ? 2 : c == ' ' ? 0 : 5; break;
        case 4: st = c == ' ' ? 0 : 1; words += 2; break;
        case 5: st = c == ',' ? 4 : 5; words--; break;
        }
    }
    return words + st;
}
