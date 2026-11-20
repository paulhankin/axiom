/* A state machine: a CSV-like lexer driven by a state variable and character classes. */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
enum { S_START, S_FIELD, S_QUOTED, S_QUOTE_IN, S_NUM, S_NUM_FRAC };
int main(void) {
    size_t n = 3 << 20;
    char *buf = malloc(n);
    uint32_t s = 99;
    for (size_t i = 0; i < n; i++) {
        s = s * 1103515245 + 12345;
        uint32_t r = (s >> 16) % 40;
        buf[i] = r < 10 ? '0' + r : r < 30 ? 'a' + (r - 10) : r < 34 ? ',' : r < 35 ? '\n' : r < 37 ? '"' : r < 38 ? '.' : ' ';
    }
    uint64_t fields = 0, rows = 0, nums = 0, fracs = 0, quoted = 0, numval = 0;
    for (int rep = 0; rep < 30; rep++) {
        int st = S_START;
        for (size_t i = 0; i < n; i++) {
            char c = buf[i];
            switch (st) {
            case S_START:
                if (c == '"') { st = S_QUOTED; quoted++; }
                else if (c >= '0' && c <= '9') { st = S_NUM; numval += c - '0'; }
                else if (c == ',') fields++;
                else if (c == '\n') { fields++; rows++; }
                else st = S_FIELD;
                break;
            case S_FIELD:
                if (c == ',') { fields++; st = S_START; }
                else if (c == '\n') { fields++; rows++; st = S_START; }
                break;
            case S_QUOTED:
                if (c == '"') st = S_QUOTE_IN;
                break;
            case S_QUOTE_IN:
                if (c == '"') st = S_QUOTED;
                else if (c == ',') { fields++; st = S_START; }
                else if (c == '\n') { fields++; rows++; st = S_START; }
                else st = S_FIELD;
                break;
            case S_NUM:
                if (c >= '0' && c <= '9') numval = numval * 3 + (c - '0');
                else if (c == '.') st = S_NUM_FRAC;
                else if (c == ',') { nums++; fields++; st = S_START; }
                else if (c == '\n') { nums++; fields++; rows++; st = S_START; }
                else st = S_FIELD;
                break;
            case S_NUM_FRAC:
                if (c >= '0' && c <= '9') fracs++;
                else if (c == ',') { nums++; fields++; st = S_START; }
                else if (c == '\n') { nums++; fields++; rows++; st = S_START; }
                else st = S_FIELD;
                break;
            }
        }
    }
    printf("fsm: %llu %llu %llu %llu %llu %llu\n", (unsigned long long)fields, (unsigned long long)rows,
           (unsigned long long)nums, (unsigned long long)fracs, (unsigned long long)quoted, (unsigned long long)numval);
    return 0;
}
