/* A bytecode interpreter: a switch over opcodes in a loop, a small stack. */
#include <stdio.h>
#include <stdint.h>
enum { PUSH, LOAD, STORE, ADD, SUB, MUL, AND, XOR, SHR, JNZ, DEC, DUP, POP, HALT };
typedef struct { uint8_t op; int32_t arg; } Ins;
static int64_t run(const Ins *code, int64_t *vars) {
    int64_t st[64];
    int sp = 0, pc = 0;
    for (;;) {
        Ins in = code[pc++];
        switch (in.op) {
        case PUSH: st[sp++] = in.arg; break;
        case LOAD: st[sp++] = vars[in.arg]; break;
        case STORE: vars[in.arg] = st[--sp]; break;
        case ADD: sp--; st[sp - 1] += st[sp]; break;
        case SUB: sp--; st[sp - 1] -= st[sp]; break;
        case MUL: sp--; st[sp - 1] *= st[sp]; break;
        case AND: sp--; st[sp - 1] &= st[sp]; break;
        case XOR: sp--; st[sp - 1] ^= st[sp]; break;
        case SHR: sp--; st[sp - 1] = (int64_t)((uint64_t)st[sp - 1] >> st[sp]); break;
        case JNZ: if (st[--sp]) pc = in.arg; break;
        case DEC: vars[in.arg]--; break;
        case DUP: st[sp] = st[sp - 1]; sp++; break;
        case POP: sp--; break;
        case HALT: return vars[0];
        }
    }
}
int main(void) {
    /* v0 = acc, v1 = counter, v2 = x
       loop: x = (x*6364136223846793005 + 1442695040888963407); acc ^= x >> 29; acc += x & 255; counter--; jnz loop */
    Ins code[] = {
        {LOAD, 2}, {PUSH, 1103515245}, {MUL, 0}, {PUSH, 12345}, {ADD, 0}, {DUP, 0}, {STORE, 2},
        {PUSH, 29}, {SHR, 0}, {LOAD, 0}, {XOR, 0}, {LOAD, 2}, {PUSH, 255}, {AND, 0}, {ADD, 0},
        {STORE, 0}, {DEC, 1}, {LOAD, 1}, {JNZ, 0}, {HALT, 0},
    };
    int64_t vars[4] = {0, 18000000, 7, 0};
    int64_t r = run(code, vars);
    printf("interp: %lld\n", (long long)r);
    return 0;
}
